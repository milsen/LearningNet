<?php

namespace LearningNet;

/**
 * Manages the database tables associated with condition ids.
 * Can retrieve and condition values for a given user and translate these values
 * from ids to actually meaningful titles.
 *
 * @author  <milsen@uos.de>
 */
class ConditionHandler
{
    /**
     * Mapping from conditions ids to a dictionary containing the database table
     * and row from which to get the corresponding condition value.
     * translation_table and translation_key give the table and row with the
     * human-readable title of the condition value.
     *
     * 'key' must be a row of 'translation_table' as well if the translation
     * should succeed.
     *
     * IMPORTANT:
     * If CONDITION_MAP is updated, getConditionTitles should be updated, too!
     */
    const CONDITION_MAP = [
        0 => [
            'table' => 'user_info',
            'key' => 'preferred_language'
        ],
        1 => [
            'table' => 'user_studiengang',
            'key' => 'fach_id',
            'translation_table' => 'fach',
            'translation_key' => 'name'
        ],
        2 => [
            'table' => 'user_studiengang',
            'key' => 'abschluss_id',
            'translation_table' => 'abschluss',
            'translation_key' => 'name'
        ],
        3 => [
            'table' => 'user_inst',
            'key' => 'Institut_id',
            'translation_table' => 'Institute',
            'translation_key' => 'Name'
        ]
    ];

    /**
     * @return array the condition property associated with each condition id
     */
    public static function getConditionTitles()
    {
        return [
            "0" => _ln('Bevorzugte Sprache'),
            "1" => _ln('Studienfach'),
            "2" => _ln('Studienabschluss'),
            "3" => _ln('Institut')
        ];
    }

    /**
     * The condition value representing the "else" branch
     */
    const CONDITION_ELSE_BRANCH_KEYWORD = "SONST";

    /* Get User Values for Conditions */

    /**
     * @param string $userId id of the user
     * @return array from condition ids to arrays of condition values
     * corresponding to the given user
     */
    public function getConditionValues($userId)
    {
        $conditionVals = [];
        foreach (array_keys(self::CONDITION_MAP) as $conditionId) {
            $conditionVals[$conditionId] =
                $this->getValuesByConditionID($conditionId, $userId);
        }
        return $conditionVals;
    }

    /**
     * @param string $conditionId condition id
     * @param string $userId id of the user
     * @return string[] condition values for the given condition id
     * corresponding to the given user
     */
    public function getValuesByConditionID($conditionId, $userId)
    {
        $condition = self::CONDITION_MAP[$conditionId];
        if ($condition === null) {
            return [];
        }

        // Key and table are not bound using bindParam since they should not be
        // packed into strings. This is only fine as long as they are hardcoded
        // above and not given by user-input!
        $db = \DBManager::get();
        $stmt = $db->prepare("SELECT {$condition['key']} "
            . "FROM {$condition['table']} "
            . "WHERE user_id = :userid "
        );
        $stmt->bindParam(':userid', $userId);
        $stmt->execute();

        $values = [];
        while ($row = $stmt->fetch(\PDO::FETCH_NUM, \PDO::FETCH_ORI_NEXT)) {
            array_push($values, strval($row[0]));
        }
        // There might be multiple values, e.g. one student may be part of
        // multiple institutes.
        return $values;
    }

    /* Translate Condition Branches */

    /**
     * Translate condition values into their humand-readable names.
     *
     * @param array $conditionIdToBranches [ conditionId => [branchname] ]
     * @return array [ conditionId => [branchname => translatedBranchName] ]
     */
    public function getConditionBranches($conditionIdToBranches)
    {
        $translations = [];

        foreach ($conditionIdToBranches as $conditionId => $branches) {
            $translations[$conditionId] = [];

            foreach ($branches as $branch) {
                // Only translate if it is not the else branch.
                $translations[$conditionId][$branch] =
                    $branch === self::CONDITION_ELSE_BRANCH_KEYWORD ?
                    self::CONDITION_ELSE_BRANCH_KEYWORD :
                    $this->getBranchesByConditionID($conditionId, $branch);
            }
        }
        return $translations;
    }

    /**
     * Translate a condition value that is just an id (e.g. fach_id) into its
     * humand-readable name.
     *
     * @param string $conditionId condition id
     * @param string $value condition value to translate
     * @return string translated human-readbable name of the condition value, or
     * the empty string if no translation was possible
     */
    private function getBranchesByConditionID($conditionId, $value)
    {
        $condition = self::CONDITION_MAP[$conditionId];
        if ($condition === null) {
            return "";
        }
        if (!array_key_exists('translation_table', $condition)) {
            return $value;
        }

        // Keys and table are not bound using bindParam since they should not be
        // packed into strings. This is only fine as long as they are hardcoded
        // above and not given by user-input!
        $db = \DBManager::get();
        $stmt = $db->prepare("SELECT {$condition['translation_key']} "
            . "FROM {$condition['translation_table']} "
            . "WHERE {$condition['key']} = :value "
        );
        $stmt->bindParam(':value', $value);
        $stmt->execute();

        $values = [];
        while ($row = $stmt->fetch(\PDO::FETCH_NUM, \PDO::FETCH_ORI_NEXT)) {
            array_push($values, $row[0]);
        }
        // There should not be multiple values, i.e. :key should be the single
        // primary key, otherwise there should not be a translation_table.
        return $values[0];
    }
}
