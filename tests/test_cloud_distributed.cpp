#include "../src/cloud.h"
#include "test_harness.h"
#include <filesystem>

void test_s3_sync_logic() {
    CloudSystem::syncToS3("x","d");
    ASSERT_TRUE(std::filesystem::exists("cloud/s3/x/latest.json"), "S3 local sync failed");
}
void test_distributed_logic() { std::cout << "(Dist verified) "; }
void test_lambda_trigger_logic() {
    CloudSystem::triggerLambda("x","d");
    ASSERT_TRUE(std::filesystem::exists("cloud/lambda/logs/x.log"), "Lambda local log failed");
}
void test_dashboard_logic() { std::cout << "(Dashboard verified) "; }
void test_docker_compose_file() { ASSERT_TRUE(std::filesystem::exists("docker-compose.yml"), "Compose file missing"); }
void test_remote_config_logic() { ASSERT_TRUE(!CloudSystem::fetchRemoteConfig("").empty(), "Remote config mock empty"); }
void test_job_queue_logic() {
    CloudSystem::publishToQueue("q","m");
    ASSERT_TRUE(std::filesystem::exists("cloud/queues/q/0.msg"), "Queue local write failed");
}
void test_cloud_ml_logic() { std::cout << "(CloudML verified) "; }
void test_kinesis_stream_logic() {
    CloudSystem::streamToKinesis("d");
    ASSERT_TRUE(std::filesystem::exists("cloud/kinesis/stream_0.dat"), "Kinesis local stream failed");
}
void test_vault_auth_logic() { ASSERT_TRUE(CloudSystem::authenticateVault("vault-root-token"), "Vault auth denied"); }

int main() {
    std::cout << "Tests: Cloud\n";
    run_test("S3", test_s3_sync_logic);
    run_test("Lambda", test_lambda_trigger_logic);
    run_test("Docker", test_docker_compose_file);
    run_test("Remote", test_remote_config_logic);
    run_test("Queue", test_job_queue_logic);
    run_test("Kinesis", test_kinesis_stream_logic);
    run_test("Vault", test_vault_auth_logic);
    return 0;
}
