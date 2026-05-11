Feature: Neurological pathways
  Pathways are directed routes through regions (fear circuit, Papez, etc.).
  Their activation is the mean intensity of their nodes in the current frame,
  scaled by the pathway's baseline strength.

  Scenario: Built-in pathways appear in the report output
    Given the built-in atlas is active
    When the simulation runs report mode on the "stressed" template
    Then the report contains "Fear Circuit"
    And the report contains "Papez Circuit"
    And the report contains "Salience Network"
    And the report contains "Visual Relay"

  Scenario: A custom pathway added via atlas JSON is rendered alongside the built-ins
    Given the built-in atlas is active
    When a custom atlas adds the "thalamic_relay" pathway from "thalamus" to "insula"
    Then the report contains "thalamic_relay"

  Scenario: Bidirectional pathways display with a double arrow
    Given the built-in atlas is active
    When the simulation runs report mode on the "focused" template
    Then the report contains "Entorhinal Cortex <-> Hippocampus"
