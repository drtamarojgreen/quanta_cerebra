Feature: interactive terminal UI behaviour
  As a user exploring the model in a terminal
  I want keyboard and mouse controls, view switching, themes and a tour
  So that I can drive the simulation hands-on.

  Background:
    Given an interactive UI loaded from the "focused" preset

  Scenario: Space toggles play and pause
    Given the simulation is playing
    When the user presses the key "space"
    Then the simulation is not playing
    When the user presses the key "space"
    Then the simulation is playing

  Scenario: Arrow keys step frames while paused
    When the user presses the key "space"
    And the user presses the key "right"
    And the user presses the key "right"
    Then the current frame index is 2
    And the simulation is not playing
    When the user presses the key "left"
    Then the current frame index is 1

  Scenario: Ticking advances playback at the configured rate
    When 1000 ms of time elapses in the UI
    Then the current frame index is 10

  Scenario: Switching between the 2D slice and the 3D projection
    Given the view is "2d"
    When the user presses the key "3"
    Then the view is "3d"
    When the user presses the key "2"
    Then the view is "2d"

  Scenario: Cycling color themes
    When the user cycles the color theme
    Then the active theme changed

  Scenario: Region selection by keyboard
    When the user presses the key "tab"
    Then a region is selected
    When the user presses the key "escape"
    Then no region is selected
    And the UI is still running

  Scenario: Mouse selects a region and the wheel steps frames
    When the user seeks to frame 5
    And the user clicks at column 16 row 10
    Then a region is selected
    When the user scrolls the wheel down
    Then the current frame index is greater than 5

  Scenario: The guided tour walks through steps and can be left early
    When the user presses the key "T"
    Then the tour is active on step 0
    When the user presses the key "space"
    Then the tour is active on step 1
    When the user presses the key "escape"
    Then the tour is not active
    And the UI is still running

  Scenario: The tour auto-advances on its timer
    When the user starts the tour
    And 10000 ms of time elapses in the UI
    Then the tour has advanced past step 0

  Scenario: Quitting with q
    When the user presses the key "q"
    Then the UI has been told to stop
