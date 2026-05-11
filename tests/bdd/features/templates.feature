Feature: Brain state templates
  Predefined templates (focused, relaxed, stressed, rem_sleep) must produce
  distinct, neuroscience-plausible regional intensity signatures.

  Scenario: Focused state has elevated prefrontal cortex relative to REM sleep
    Given the four built-in brain-state templates
    Then "focused" "prefrontal_cortex" intensity is greater than "rem_sleep" "prefrontal_cortex" intensity

  Scenario: Stressed state has a much higher amygdala than relaxed
    Given the four built-in brain-state templates
    Then "stressed" "amygdala" intensity exceeds "relaxed" "amygdala" intensity by at least 0.40

  Scenario: REM sleep keeps the hippocampus elevated
    Given the four built-in brain-state templates
    Then "rem_sleep" "hippocampus" intensity is greater than 0.50
