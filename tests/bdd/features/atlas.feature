Feature: Configurable region atlas
  Users can extend or override the built-in atlas with a JSON file so they can
  add new regions or adjust geometry without recompiling Brain Modeler.

  Scenario: Adding a region via a custom atlas makes it appear in every display
    Given the built-in atlas is active
    When a custom atlas adds the "cerebellum" region with display name "Cerebellum"
    Then "Cerebellum" appears in the rendered region table
    And "Cerebellum" appears in the 2D slice legend

  Scenario: Loading an atlas resets the previous one
    Given the built-in atlas is active
    When a custom atlas with only "solo" is loaded
    Then the current atlas contains 1 region

  Scenario: Per-region metrics flow from JSON input into the report output
    Given the built-in atlas is active
    When a JSON frame supplies a "bold" metric of 0.82 on the "amygdala"
    Then the rendered region table shows "bold=0.82" next to "Amygdala"
