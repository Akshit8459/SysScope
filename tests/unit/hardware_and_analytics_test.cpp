#include "tests/test_framework.hpp"
#include "sysscope/collectors/network.hpp"
#include "sysscope/collectors/disk.hpp"
#include "sysscope/collectors/thermal.hpp"
#include "sysscope/platform/accelerator_backend.hpp"
#include "sysscope/analytics/correlation.hpp"
#include "sysscope/storage/persistence.hpp"
#include "sysscope/storage/sqlite_persistence.hpp"
#include "sysscope/ipc/ipc.hpp"
#include "sysscope/ipc/real_ipc.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include <memory>

TEST_CASE("Collectors::NetworkCollector Interface Parsing") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    std::string net_dev = 
        "Inter-|   Receive                                                |  Transmit\n"
        " face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"
        "  eth0: 10485760   10000    0    0    0     0          0         0 5242880    5000    0    0    0     0       0          0\n";

    mock_fs->set_file_content("/proc/net/dev", net_dev);

    sysscope::collectors::NetworkCollector collector(mock_fs);
    auto res = collector.collect();

    REQUIRE(res.is_ok());
    REQUIRE_EQ(res.value().interfaces.size(), 1ULL);
    REQUIRE_EQ(res.value().interfaces[0].interface_name, "eth0");
}

TEST_CASE("Collectors::ThermalCollector Dynamic Thermal Zone Mapping") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    mock_fs->add_directory_entry("/sys/class/thermal", "thermal_zone0");
    mock_fs->add_directory_entry("/sys/class/thermal", "thermal_zone1");

    mock_fs->set_file_content("/sys/class/thermal/thermal_zone0/type", "x86_pkg_temp\n");
    mock_fs->set_file_content("/sys/class/thermal/thermal_zone0/temp", "65000\n");

    mock_fs->set_file_content("/sys/class/thermal/thermal_zone1/type", "gpu_thermal\n");
    mock_fs->set_file_content("/sys/class/thermal/thermal_zone1/temp", "82000\n");

    sysscope::collectors::ThermalCollector collector(mock_fs);
    auto res = collector.collect();

    REQUIRE(res.is_ok());
    REQUIRE_EQ(res.value().zones.size(), 2ULL);

    const auto& zones = res.value().zones;
    auto z0_it = std::find_if(zones.begin(), zones.end(), [](const auto& z) { return z.name == "thermal_zone0"; });
    auto z1_it = std::find_if(zones.begin(), zones.end(), [](const auto& z) { return z.name == "thermal_zone1"; });

    REQUIRE(z0_it != zones.end());
    REQUIRE(z1_it != zones.end());

    REQUIRE_EQ(z0_it->type, "x86_pkg_temp");
    REQUIRE(z0_it->temperature_celsius >= 64.9 && z0_it->temperature_celsius <= 65.1);

    REQUIRE_EQ(z1_it->type, "gpu_thermal");
    REQUIRE(z1_it->temperature_celsius >= 81.9 && z1_it->temperature_celsius <= 82.1);
}

TEST_CASE("Analytics::CorrelationEngine Thermal Throttling Diagnosis") {
    sysscope::analytics::CorrelationEngine engine;
    sysscope::core::MetricSnapshot snapshot{};

    snapshot.cpu.total_utilization = 91.2;

    sysscope::core::GpuSnapshot gpu_snap{};
    gpu_snap.name = "NVIDIA RTX 4090";
    gpu_snap.utilization_percent = 94.5;
    gpu_snap.temperature_celsius = 82.0;
    snapshot.gpu = gpu_snap;

    sysscope::core::PsiSnapshot psi_snap{};
    psi_snap.memory_some.avg10 = 12.3;
    snapshot.psi = psi_snap;
    snapshot.memory.utilization_percent = 78.0;

    auto diagnoses = engine.analyze(snapshot);
    REQUIRE(diagnoses.size() >= 1ULL);
    REQUIRE_EQ(diagnoses[0].condition, "THERMAL THROTTLING");
    REQUIRE(diagnoses[0].confidence >= 0.85);
    REQUIRE(diagnoses[0].evidence.size() >= 2ULL);
}

TEST_CASE("Analytics::CorrelationEngine CpuSchedulingContention Diagnosis") {
    sysscope::analytics::CorrelationEngine engine;
    sysscope::core::MetricSnapshot snapshot{};

    snapshot.cpu.total_utilization = 88.5;

    sysscope::core::PsiSnapshot psi_snap{};
    psi_snap.cpu_some.avg10 = 18.4;
    snapshot.psi = psi_snap;

    auto diagnoses = engine.analyze(snapshot);
    REQUIRE_EQ(diagnoses.size(), 1ULL);
    REQUIRE_EQ(diagnoses[0].condition, "CPU SCHEDULING CONTENTION");
    REQUIRE(diagnoses[0].confidence >= 0.85);
}

TEST_CASE("Analytics::CorrelationEngine IoBottleneck Diagnosis") {
    sysscope::analytics::CorrelationEngine engine;
    sysscope::core::MetricSnapshot snapshot{};

    snapshot.cpu.iowait_percent = 14.2;

    sysscope::core::PsiSnapshot psi_snap{};
    psi_snap.io_some.avg10 = 22.1;
    snapshot.psi = psi_snap;

    auto diagnoses = engine.analyze(snapshot);
    REQUIRE_EQ(diagnoses.size(), 1ULL);
    REQUIRE_EQ(diagnoses[0].condition, "I/O BOTTLENECK");
    REQUIRE(diagnoses[0].confidence >= 0.85);
}

TEST_CASE("Storage::MockPersistenceEngine Storage and Querying") {
    sysscope::storage::MockPersistenceEngine engine;
    REQUIRE(engine.initialize("mock_sysscope.db"));

    sysscope::core::MetricSnapshot snapshot{};
    snapshot.timestamp = sysscope::core::Timestamp::now();
    snapshot.cpu.total_utilization = 45.2;

    auto store_res = engine.store_snapshot(snapshot);
    REQUIRE(store_res.is_ok());

    sysscope::analytics::Diagnosis diag{};
    diag.condition = "CPU SCHEDULING CONTENTION";
    diag.confidence = 0.90;

    auto diag_res = engine.store_diagnosis(diag, snapshot.timestamp.nanoseconds());
    REQUIRE(diag_res.is_ok());

    auto hist_res = engine.query_history("cpu_utilization", 0, snapshot.timestamp.nanoseconds() + 1000);
    REQUIRE(hist_res.is_ok());
    REQUIRE_EQ(hist_res.value().size(), 1ULL);
    REQUIRE_EQ(hist_res.value()[0].metric_type, "cpu_utilization");
    REQUIRE(hist_res.value()[0].value >= 45.1 && hist_res.value()[0].value <= 45.3);

    auto diag_hist_res = engine.query_diagnoses(0, snapshot.timestamp.nanoseconds() + 1000);
    REQUIRE(diag_hist_res.is_ok());
    REQUIRE_EQ(diag_hist_res.value().size(), 1ULL);
    REQUIRE_EQ(diag_hist_res.value()[0].condition, "CPU SCHEDULING CONTENTION");
}

TEST_CASE("Ipc::MockIpcSubsystem Server and Client Communication") {
    sysscope::ipc::MockIpcServer server;
    REQUIRE(server.start("/tmp/syscope.sock"));
    REQUIRE(server.is_running());

    sysscope::ipc::MockIpcClient client;
    REQUIRE(client.connect("/tmp/syscope.sock"));

    auto ping_res = client.send_request(sysscope::ipc::IpcCommandType::Ping);
    REQUIRE(ping_res.is_ok());
    REQUIRE_EQ(ping_res.value(), "PONG");

    auto snap_res = client.send_request(sysscope::ipc::IpcCommandType::GetSnapshot);
    REQUIRE(snap_res.is_ok());
    REQUIRE_EQ(snap_res.value(), "OK");

    client.disconnect();
    server.stop();
    REQUIRE(!server.is_running());
}

TEST_CASE("Platform::AcceleratorBackend Qualcomm and NVIDIA Backends") {
    sysscope::platform::QualcommAcceleratorBackend qcom_backend;
    REQUIRE_EQ(qcom_backend.name(), "Qualcomm Adreno/Hexagon NPU/DSP Backend");
    REQUIRE(!qcom_backend.is_available());
    auto q_res = qcom_backend.collect();
    REQUIRE(q_res.is_err());

    sysscope::platform::NvidiaGpuBackend nv_backend;
    REQUIRE_EQ(nv_backend.name(), "NVIDIA NVML Backend");
    REQUIRE(!nv_backend.is_available());
    auto nv_res = nv_backend.collect();
    REQUIRE(nv_res.is_err());
}

TEST_CASE("Storage::SqlitePersistenceEngine Storage and TimeSeries Queries") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();
    sysscope::storage::SqlitePersistenceEngine engine(mock_fs);

    REQUIRE(engine.initialize("/tmp/test_sysscope.db"));

    sysscope::core::MetricSnapshot snapshot{};
    snapshot.timestamp = sysscope::core::Timestamp::now();
    snapshot.cpu.total_utilization = 52.4;

    auto store_res = engine.store_snapshot(snapshot);
    REQUIRE(store_res.is_ok());

    auto query_res = engine.query_history("cpu_utilization", 0, snapshot.timestamp.nanoseconds() + 1000);
    REQUIRE(query_res.is_ok());
    REQUIRE_EQ(query_res.value().size(), 1ULL);
    REQUIRE(query_res.value()[0].value >= 52.3 && query_res.value()[0].value <= 52.5);
}

TEST_CASE("Ipc::RealIpcSubsystem Real Socket Connection") {
    sysscope::ipc::RealIpcServer server;
    REQUIRE(server.start("/tmp/syscope_real.sock"));
    REQUIRE(server.is_running());

    sysscope::ipc::RealIpcClient client;
    REQUIRE(client.connect("/tmp/syscope_real.sock"));

    auto ping_res = client.send_request(sysscope::ipc::IpcCommandType::Ping);
    REQUIRE(ping_res.is_ok());
    REQUIRE_EQ(ping_res.value(), "PONG");

    client.disconnect();
    server.stop();
    REQUIRE(!server.is_running());
}
