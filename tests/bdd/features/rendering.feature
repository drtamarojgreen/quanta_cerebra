Feature: Rendering
  The 2D coronal slice, 3D ASCII projection, and region table must label every
  region of interest and scale with the requested terminal width.

  Scenario: Each frame renders all region-of-interest labels
    Given a "focused" template frame at timestamp 0
    And the "classic" theme is selected
    When the 2D slice is rendered at width 100
    Then the slice output contains every region display name
    When the 3D projection is rendered at 80 by 20
    Then the projection canvas contains at least 200 characters
    When the region table is rendered at width 80
    Then the table output contains every region display name

  Scenario: Output scales with the requested terminal width
    Given a "stressed" template frame at timestamp 0
    And the "matrix" theme is selected
    When the 2D slice is rendered at width 50
    And the 2D slice is rendered again at width 160
    Then the wider rendering produces longer output
