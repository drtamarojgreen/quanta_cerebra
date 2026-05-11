Feature: simulation playback control
  As a researcher reviewing recorded brain activity
  I want to pause, step, rewind and fast-forward the timeline
  So that I can study individual moments in detail.

  Scenario: Pausing and resuming playback
    Given a loaded simulation with 30 frames at 10 fps
    And the simulation is playing
    When the user pauses and 2000 ms of time passes
    Then the current frame index is 0
    When the user resumes and 100 ms of time passes
    Then the current frame index is 1

  Scenario: Rewinding and fast-forwarding
    Given a loaded simulation with 30 frames at 10 fps
    When the simulation is paused
    And the user seeks to frame 15
    And the user fast-forwards 10 frames
    Then the current frame index is 25
    When the user rewinds 100 frames
    Then the current frame index is 0

  Scenario: Playback stops at the end of the timeline
    Given a loaded simulation with 10 frames at 10 fps
    And the simulation is playing
    When 100000 ms of time passes
    Then the current frame index is the last frame
    And the simulation is not playing

  Scenario: Inspecting a region over the run
    Given a simulation built from the "stressed" preset
    When the user selects the "amygdala" region
    Then the selected region is "amygdala"
    And the chemistry is stable after seeking to the end
