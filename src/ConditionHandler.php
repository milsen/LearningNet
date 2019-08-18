<?php

namespace LearningNet;

use LearningNet\DataExtractor;

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

    private $conditionVals;

    public function __construct($network, $userId)
    {
        $this->conditionVals = array();
        $extractor = new DataExtractor();
        foreach ($extractor->extractUsedConditions($network) as $conditionId) {
            $this->conditionVals[$conditionId] =
                $this->getValues($conditionId, $userId);
        }
    }

    public function getValues($conditionId, $userId)
    {
        $condition = self::CONDITION_MAP[$conditionId];

        // Key and table are not bound using bindParam since they should not be
        // packed into strings. This is only fine as long as they are hardcoded
        // above and not given by user-input!
        $db = \DBManager::get();
        $stmt = $db->prepare("
            SELECT
                " . $condition['key'] . "
            FROM
                " . $condition['table'] . "
            WHERE
                user_id = :userid
        ");
        $stmt->bindParam(':userid', $userId);
        $stmt->execute();

        $values = array();
        while ($row = $stmt->fetch(\PDO::FETCH_NUM, \PDO::FETCH_ORI_NEXT)) {
            array_push($values, $row[0]);
        }
        // There might be multiple values, e.g. one student may be part of
        // multiple institutes.
        return $values;
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
        while ($row = $stmt->fetch(\PDO::FETCH_NUM, \PDO::FETCH_ORI_NEXT)) {
            array_push($values, $row[0]);
        }
        // There should not be multiple values, i.e. :key should be the single
        // primary key, otherwise there should not be a translation_table.
        return $values[0];
    }
}
