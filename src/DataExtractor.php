<?php

namespace LearningNet;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class DataExtractor
{
    // Type id for condition nodes.
    const CONDITION_TYPE = 11;
    // Header strings used for the columns of node types.
    const TYPE_HEADER = "type";
    // Header strings used for the columns of node references.
    const REF_HEADER = "ref";

    public function __construct()
    { }

    public function extractConditions($network)
    {
        $conditionIds = array();

        $read = "nodeSection";
        $typeColumn = 0;
        $refColumn = 0;

        // For every non-empty non-comment line.
        foreach (explode("\n", $network) as $line) {
            $line = trim($line);

            if ($line !== "" && $line[0] !== "#") {
                if ($line[0] === "@" && $line !== "@nodes") {
                    // If we see a non-node section, wait again until the
                    // nodeSection arrives.
                    $read = "nodeSection";
                } else if ($read === "nodeSection" && $line === "@nodes") {
                    $read = "nodeHeader";
                } else if ($read === "nodeHeader") {
                    $headers = preg_split("/\s+/", $line);
                    $typeColumn = array_search(self::TYPE_HEADER, $headers);
                    $refColumn = array_search(self::REF_HEADER, $headers);
                    $read = "rows";
                } else if ($read === "rows") {
                    $row = preg_split("/\s+/", $line);
                    if (intval($row[$typeColumn]) === self::CONDITION_TYPE) {
                        array_push($conditionIds, intval($row[$refColumn]));
                    }
                }
            }
        }
        return array_unique($conditionIds);
    }
}
