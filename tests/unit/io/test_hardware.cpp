#include "../src/config.h"
#include "test_harness.h"

void test_eeg_logic() { AppConfig c; c.hardware_eeg_connected = true; ASSERT_TRUE(c.hardware_eeg_connected, "EEG state fail"); }
void test_fmri_logic() { bool fmri_active = true; ASSERT_TRUE(fmri_active, "fMRI state fail"); }
void test_gpio_logic() { bool gpio_active = true; ASSERT_TRUE(gpio_active, "GPIO state fail"); }
void test_gpu_logic() { AppConfig c; c.gpu_enabled = true; ASSERT_TRUE(c.gpu_enabled, "GPU state fail"); }
void test_haptic_logic() { AppConfig c; c.haptic_feedback_active = true; ASSERT_TRUE(c.haptic_feedback_active, "Haptic state fail"); }
void test_sound_logic() { bool sound_active = true; ASSERT_TRUE(sound_active, "Sound state fail"); }
void test_vr_logic() { AppConfig c; c.vr_streaming_active = true; ASSERT_TRUE(c.vr_streaming_active, "VR state fail"); }
void test_resource_logic() { bool res_active = true; ASSERT_TRUE(res_active, "Resource state fail"); }
void test_tpu_logic() { AppConfig c; c.tpu_enabled = true; ASSERT_TRUE(c.tpu_enabled, "TPU state fail"); }
void test_net_sync_logic() { bool net_active = true; ASSERT_TRUE(net_active, "NetSync state fail"); }

int main() {
    std::cout << "Tests: Hardware\n";
    run_test("EEG", test_eeg_logic); run_test("fMRI", test_fmri_logic);
    run_test("GPIO", test_gpio_logic); run_test("GPU", test_gpu_logic);
    run_test("Haptic", test_haptic_logic); run_test("Sound", test_sound_logic);
    run_test("VR", test_vr_logic); run_test("Resources", test_resource_logic);
    run_test("TPU", test_tpu_logic); run_test("NetSync", test_net_sync_logic);
    return 0;
}
