Feature: non-interactive report output
  As a user piping the tool into a log or notebook
  I want a self-contained text report
  So that I can read the simulation result without a terminal UI.

  Scenario: Generating a report from a JSON activity file
    Given a temporary JSON activity file with 2 frames
    When the tool runs in report mode against that file
    Then the run exits successfully
    And the report contains "simulation report"
    And the report contains "frames      : 2"
    And the report contains "Region activity"
    And the report contains "Amygdala"
    And the report contains "Neurochemistry"
    And the report contains "Coronal slice"
    And the report contains "intensity legend"

  Scenario: Generating a 3D report from a brain-state preset
    Given report options for the "rem_sleep" preset in 3d view with 12 frames
    When the report is produced
    Then the run exits successfully
    And the report contains "frames      : 12"
    And the report contains "3D projection"
    And the report contains "Hippocampus"

  Scenario: A missing input file is reported as an error
    Given report options for the input file "/no/such/file-xyz.json"
    When the report is produced
    Then the run exits with a non-zero code

  Scenario: An ASCII report is pure ASCII and lists every region with its ROI flag
    Given report options for the "focused" preset in 2d view with 8 frames
    When the report is produced
    Then the run exits successfully
    And the report contains only ASCII bytes
    And every report line is at most 72 columns wide
    And the report flags region "Amygdala" as a region of interest
    And the report flags region "Occipital Lobe" as not a region of interest
    And the report contains "Cingulate Gyrus"
