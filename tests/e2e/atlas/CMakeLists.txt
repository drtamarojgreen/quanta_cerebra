configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sample_frames.json
               ${CMAKE_CURRENT_BINARY_DIR}/sample_frames.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/malformed.json
               ${CMAKE_CURRENT_BINARY_DIR}/malformed.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/examples/custom_atlas.json
               ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json COPYONLY)

# ----- Happy path -----

add_test(
    NAME e2e_report_template
    COMMAND $<TARGET_FILE:brain_modeler> --report --template focused --theme mono
)
set_tests_properties(e2e_report_template PROPERTIES
    PASS_REGULAR_EXPRESSION "Prefrontal Cortex")

add_test(
    NAME e2e_report_json
    COMMAND $<TARGET_FILE:brain_modeler>
            --report --input ${CMAKE_CURRENT_BINARY_DIR}/sample_frames.json
            --theme mono
)
set_tests_properties(e2e_report_json PROPERTIES
    PASS_REGULAR_EXPRESSION "Amygdala")

add_test(NAME e2e_help COMMAND $<TARGET_FILE:brain_modeler> --help)
set_tests_properties(e2e_help PROPERTIES
    PASS_REGULAR_EXPRESSION "Brain Modeler")

add_test(NAME e2e_tour COMMAND $<TARGET_FILE:brain_modeler> --tour --theme mono)
set_tests_properties(e2e_tour PROPERTIES
    PASS_REGULAR_EXPRESSION "tour")

# ----- Thorough report-content checks -----

# Each test below greps for a specific string in the report output. Splitting
# them keeps failures attributable to a single missing element.

function(add_report_content_test name needle)
    add_test(
        NAME ${name}
        COMMAND bash -c
        "$<TARGET_FILE:brain_modeler> --report --template focused --theme mono | grep -qF '${needle}'"
    )
endfunction()

add_report_content_test(e2e_report_has_prefrontal       "Prefrontal Cortex")
add_report_content_test(e2e_report_has_amygdala         "Amygdala")
add_report_content_test(e2e_report_has_hippocampus      "Hippocampus")
add_report_content_test(e2e_report_has_thalamus         "Thalamus")
add_report_content_test(e2e_report_has_insula           "Insula")
add_report_content_test(e2e_report_has_acc              "Anterior Cingulate Cortex")
add_report_content_test(e2e_report_has_entorhinal       "Entorhinal Cortex")
add_report_content_test(e2e_report_has_cuneus           "Cuneus")
add_report_content_test(e2e_report_has_occipital        "Occipital Lobe")
add_report_content_test(e2e_report_has_gyrus            "Cingulate Gyrus")
add_report_content_test(e2e_report_has_first_frame      "frame 1/30")
add_report_content_test(e2e_report_has_last_frame       "frame 30/30")
add_report_content_test(e2e_report_has_neurotransmitter "Neurotransmitters")
add_report_content_test(e2e_report_has_slice_section    "2D coronal slice")
add_report_content_test(e2e_report_has_projection       "3D ASCII projection")
add_report_content_test(e2e_report_has_theme            "theme=mono")

# Negative content checks: report mode must not leak interactive chrome.
function(add_report_absence_test name needle)
    add_test(
        NAME ${name}
        COMMAND bash -c
        "! $<TARGET_FILE:brain_modeler> --report --template focused --theme mono | grep -qF '${needle}'"
    )
endfunction()

add_report_absence_test(e2e_report_no_pause_hint    "[space]")
add_report_absence_test(e2e_report_no_paused_label  "PAUSED")
add_report_absence_test(e2e_report_no_speed_label   "speed=")

# JSON input flows through report mode end-to-end.
add_test(
    NAME e2e_report_json_timestamps
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --report --input ${CMAKE_CURRENT_BINARY_DIR}/sample_frames.json --theme mono | grep -qF 't=0.20s' && $<TARGET_FILE:brain_modeler> --report --input ${CMAKE_CURRENT_BINARY_DIR}/sample_frames.json --theme mono | grep -qF 't=0.40s'"
)

# ----- CLI failure modes -----

# Exit-code checks
add_test(NAME e2e_bad_input
    COMMAND $<TARGET_FILE:brain_modeler> --report --input /no/such/file.json)
set_tests_properties(e2e_bad_input PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_bad_template
    COMMAND $<TARGET_FILE:brain_modeler> --report --template nonsense)
set_tests_properties(e2e_bad_template PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_unknown_flag
    COMMAND $<TARGET_FILE:brain_modeler> --bogus-flag)
set_tests_properties(e2e_unknown_flag PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_missing_arg_input
    COMMAND $<TARGET_FILE:brain_modeler> --input)
set_tests_properties(e2e_missing_arg_input PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_missing_arg_template
    COMMAND $<TARGET_FILE:brain_modeler> --template)
set_tests_properties(e2e_missing_arg_template PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_malformed_json
    COMMAND $<TARGET_FILE:brain_modeler> --report
            --input ${CMAKE_CURRENT_BINARY_DIR}/malformed.json)
set_tests_properties(e2e_malformed_json PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_serial_bad_device
    COMMAND $<TARGET_FILE:brain_modeler> --report --serial /no/such/device)
set_tests_properties(e2e_serial_bad_device PROPERTIES WILL_FAIL TRUE)

# Error-message checks (stderr).
add_test(NAME e2e_bad_template_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --report --template nonsense 2>&1 | grep -qF 'Unknown template'")

add_test(NAME e2e_unknown_flag_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --bogus-flag 2>&1 | grep -qF 'Unknown argument'")

add_test(NAME e2e_missing_arg_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --input 2>&1 | grep -qF 'Missing value'")

add_test(NAME e2e_malformed_json_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --report --input ${CMAKE_CURRENT_BINARY_DIR}/malformed.json 2>&1 | grep -qF 'Failed to load input'")

add_test(NAME e2e_serial_bad_device_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --report --serial /no/such/device 2>&1 | grep -qF 'Could not open serial port'")

add_test(NAME e2e_bad_input_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --report --input /no/such/file.json 2>&1 | grep -qF 'Failed to load input'")

# ----- Custom atlas -----

# Custom atlas adds the "cerebellum" region; report mode should label it.
add_test(NAME e2e_atlas_adds_region
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --report --template focused --theme mono | grep -qF 'Cerebellum'")

# Custom atlas overrides the amygdala's display name.
add_test(NAME e2e_atlas_overrides_region
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --report --template stressed --theme mono | grep -qF 'Amygdala (lateral)'")

# Custom transmitters introduced by the atlas surface in the table.
add_test(NAME e2e_atlas_custom_transmitter
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --report --template stressed --theme mono | grep -qF 'corticotropin_releasing_factor'")

# Bad atlas path fails with a clear error.
add_test(NAME e2e_atlas_missing_file
    COMMAND $<TARGET_FILE:brain_modeler> --atlas /no/such/atlas.json --report --template focused)
set_tests_properties(e2e_atlas_missing_file PROPERTIES WILL_FAIL TRUE)

add_test(NAME e2e_atlas_missing_file_message
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas /no/such/atlas.json --report --template focused 2>&1 | grep -qF 'Failed to load atlas'")

# ----- Built-in pathways -----

function(add_report_pathway_test name needle)
    add_test(
        NAME ${name}
        COMMAND bash -c
        "$<TARGET_FILE:brain_modeler> --report --template stressed --theme mono | grep -qF '${needle}'"
    )
endfunction()

add_report_pathway_test(e2e_pathway_fear        "Fear Circuit")
add_report_pathway_test(e2e_pathway_papez       "Papez Circuit")
add_report_pathway_test(e2e_pathway_salience    "Salience Network")
add_report_pathway_test(e2e_pathway_visual      "Visual Relay")
add_report_pathway_test(e2e_pathway_arrow       " -> ")
add_report_pathway_test(e2e_pathway_double_arrow " <-> ")

# Custom atlas adds new pathways and brings in a custom transmitter on one of
# them.
add_test(NAME e2e_atlas_adds_pathway
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --report --template stressed --theme mono | grep -qF 'Stria Terminalis'")

add_test(NAME e2e_atlas_cerebellar_pathway
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --report --template stressed --theme mono | grep -qF 'Cerebellar-Thalamic Loop'")

# ----- Atlas-defined custom templates -----

# A template defined only in the custom atlas can be selected via --template.
add_test(NAME e2e_atlas_template_meditation
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --template meditation --report --theme mono | grep -qF 'Cerebellum'")

# meditation has anterior_cingulate=0.70; with 10 hashes per intensity, the
# bar for ACC must therefore include the seven-hash band.
add_test(NAME e2e_atlas_template_meditation_intensities
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --template meditation --report --theme mono | grep -qE 'Anterior Cingulate Cortex +0\\.70'")

# The custom atlas overrides the built-in "stressed" template — the override
# pushes amygdala to 1.00, which the legacy hard-coded value never reached.
add_test(NAME e2e_atlas_template_override_stressed
    COMMAND bash -c
    "$<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --template stressed --report --theme mono | grep -qE 'Amygdala[^0-9]+1\\.00'")

# Unknown template against custom atlas still produces a friendly error.
add_test(NAME e2e_atlas_template_unknown
    COMMAND $<TARGET_FILE:brain_modeler> --atlas ${CMAKE_CURRENT_BINARY_DIR}/custom_atlas.json --template flow_state --report)
set_tests_properties(e2e_atlas_template_unknown PROPERTIES WILL_FAIL TRUE)

# ----- PTY-driven end-to-end (POSIX only) -----

if(UNIX AND NOT APPLE)
    add_executable(pty_serial_fixture pty_serial_fixture.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(pty_serial_fixture PRIVATE Threads::Threads)

    add_test(
        NAME e2e_serial_pty_three_frames
        COMMAND $<TARGET_FILE:pty_serial_fixture> $<TARGET_FILE:brain_modeler>
    )
endif()
