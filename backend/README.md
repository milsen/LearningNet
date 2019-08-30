Pathfinder uses a JSON object as input, with the following keys:

* action: ["check", "create", "recommend"],
* recType (for "recommend"): ["active", "next", "path"]
    For every recType, the full learning net with active nodes set is written to stdout.
    If "active" is given, the path attribute is not set.
    If "next" is given, the recommended-attribute is set to one recommended unit node.
    If "path" is given, the recommended-attribute is set to a sequence of recommended node.
* network (for "check", "recommend"): Network as string.
* sections (for "create", "recommend"): Relevant sections as space-separated string.
    Marks completed sections for "recommend".
* conditionValues (for "recommend"): Array using conditionIds as indices, of the form
    [[conditionBranchNames]]
  for each conditionId.
* testGrades (for "recommend"): Object of the form  { testId : grade }.
* nodeCosts (for "recommend", "next" or "path"): Array of the form
    [ { "weight" : weight, "costs" : { sectionId : costValue } ].
* nodePairCosts (for "recommend", "next" or "path"): Array of the form
    [ { "weight" : weight, "costs" : { sectionId : { sectionId : costValue }} ].
