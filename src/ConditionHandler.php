<?php

namespace LearningNet;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class ConditionHandler
{
    // There should not be multiple values, i.e. :key should be the single
    // primary key, otherwise there should not be a translation_table.
    // TODO gettext
    const CONDITION_MAP = [
        1 => [
            'name' => 'Bevorzugte Sprache',
            'table' => 'user_info',
            'key' => 'preferred_language'
        ],
        2 => [
            'name' => 'Studienfach',
            'table' => 'user_studiengang',
            'key' => 'fach_id',
            'translation_table' => 'fach',
            'translation_key' => 'name',
        ],
        3 => [
            'name' => 'Studienabschluss',
            'table' => 'user_studiengang',
            'key' => 'abschluss_id'
        ],
        4 => [
            'name' => 'Institut',
            'table' => 'user_inst',
            'key' => 'Institut_id'
        ]
    ];

    // Condition id that signifies no condition.
    const NONE_CONDITION = 0;
    // Type id for split nodes.
    const SPLIT_TYPE = 10;
    // Header strings used for the columns of node types.
    const TYPE_HEADER = "type";
    // Header strings used for the columns of node references.
    const REF_HEADER = "ref";

    private $conditionVals;

    public function __construct($network, $courseId, $userId)
    {
        $this->conditionVals = array();
        foreach ($this->extractUsedConditions($network) as $conditionId) {
            $this->conditionVals[$conditionId] =
                $this->getValues($conditionId, $courseId, $userId);
        }
    }

    public function getValues($conditionId, $courseId, $userId)
    {
        $condtion = self::CONDITION_MAP[$conditionId];

        $db = \DBManager::get();
        $stmt = $db->prepare("
            SELECT
                :key
            FROM
                :table
            WHERE
                user_id = :userid
            AND
                seminar_id = :cid
        ");
        $stmt->bindParam(':key', $condition['key']);
        $stmt->bindParam(':table', $condition['table']);
        $stmt->bindParam(':userid', $userId);
        $stmt->bindParam(':cid', $courseId);
        $stmt->execute();

        $values = array();
        while ($row = $stmt->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_NEXT)) {
            array_push($values, $row[0]);
        }
        // There might be multiple values, e.g. one student may be part of
        // multiple institutes.
        return $values;
    }

    public function extractUsedConditions($network)
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
                    if ($row[$typeColumn] === self::SPLIT_TYPE &&
                        $row[$refColumn] !== self::NONE_CONDITION) {
                        array_push($conditionIds, $row[$refColumn]);
                    }
                }
            }
        }
        return array_unique($conditionIds);
    }

    public function getConditionVals()
    {
        return $this->conditionVals;
    }

    // For AJAX stuff
    public function getConditionName($conditionId)
    {
        return self::CONDITION_MAP[$conditionId]['name'];
    }

    /**
     * Translate id (e.g. fach_id) into its name
     */
    public function getValueName($conditionId, $value)
    {
        $condition = self::CONDITION_MAP[$conditionId];
        if (!array_key_exists('translation_table', $condition)) {
            return $value;
        }

        $db = \DBManager::get();
        $stmt = $db->prepare("
            SELECT
                :translation_key
            FROM
                :translation_table
            WHERE
                :key = :value
        ");
        $stmt->bindParam(':translation_key', $condition['translation_key']);
        $stmt->bindParam(':translation_table', $condition['translation_table']);
        $stmt->bindParam(':key', $condition['key']);
        $stmt->bindParam(':value', $value);
        $stmt->execute();

        $values = array();
        while ($row = $stmt->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_NEXT)) {
            array_push($values, $row[0]);
        }
        // There should not be multiple values, i.e. :key should be the single
        // primary key, otherwise there should not be a translation_table.
        return $values[0];
    }
}
