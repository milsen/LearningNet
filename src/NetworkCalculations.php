<?php

namespace LearningNet;

/**
 * PHP interface for the backend.
 * Calls the backend executable and returns its output.
 *
 * @author  <milsen@uos.de>
 */
class NetworkCalculations
{
    // Path of the backend executable relative to the pluging path.
    const EXE_PATH = '/backend/build/learningnet-pathfinder';

    /* @var string full path of the backend executable */
    private static $executablePath = "";

    /**
     * Constructs a NetworkCalculations interface.
     * @param string $pluginPath path to the LearningNet plugin
     */
    public function __construct($pluginPath)
    {
        $this->executablePath = $pluginPath . self::EXE_PATH;
    }

    /**
     * Builds a shell command calling the backend executable with the JSON
     * representation of the given arg.
     *
     * @param mixed[] $arg array whose JSON representation is passed over to the
     * backend executable
     * @return string shell command calling the backend executable with $arg
     */
    private function buildCommand($arg)
    {
        return $this->executablePath . ' \'' . json_encode($arg) . '\' 2>&1';
    }

    /**
     * Runs the backend executable with the JSON representation of the given arg.
     *
     * @param mixed[] $arg array whose JSON representation is passed over to the
     * backend executable
     * @return array array with the command output under the key 'message' and
     * a bool indicating whether the command succeeded under the key 'succeeded'
     */
    private function runCommand($arg)
    {
        $output = [];
        $returnVar = 0;

        exec($this->buildCommand($arg), $output, $returnVar);

        return [
            'message' => join("\n", $output),
            'succeeded' => $returnVar === 0
        ];
    }

    /**
     * Varifies the validity of a learning net.
     *
     * @param string $networkLGF LGF representation of a learning net
     * @return array array with a bool indicating whether the check succeeded
     * under the key 'succeeded' and error messages under the key 'message' if
     * the check failed
     */
    public function checkNetwork($networkLGF)
    {
        return $this->runCommand([
            'action' => 'check',
            'network' => $networkLGF
        ]);
    }

    /**
     * Get the active nodes of a learning net for a given set of already
     * completed sections, condition values and test grades.
     *
     * @param string $networkLGF LGF representation of a learning net
     * @param int[] $completedSections array of sections ids for completed sections
     * @param array[] $conditionValues array of condition values for each
     * condition id (array index)
     * @param int[] $testGrades array of test grades indexed by test block ids
     * @return array array with a bool indicating whether the command succeeded
     * under the key 'succeeded' and error messages under the key 'message' if
     * the command failed or a network with set active nodes under the key
     * 'message' if the command succeeded
     */
    public function getActives($networkLGF,
        $completedSections, $conditionValues, $testGrades)
    {
        return $this->runCommand([
            'action' => 'recommend',
            'recType' => 'active',
            'network' => $networkLGF,
            'sections' => $completedSections,
            'conditions' => $conditionValues,
            'testGrades' => $testGrades
        ]);
    }

    /**
     * Get the active nodes of a learning net and a recommended learning path
     * for a given set of already completed sections, condition values and test
     * grades.
     *
     * @param string $networkLGF LGF representation of a learning net
     * @param int[] $completedSections array of sections ids for completed sections
     * @param array[] $conditionValues array of condition values for each
     * condition id (array index)
     * @param int[] $testGrades array of test grades indexed by test block ids
     * @return array array with a bool indicating whether the command succeeded
     * under the key 'succeeded' and error messages under the key 'message' if
     * the command failed or a network with set active nodes and learning path
     * under the key 'message' if the command succeeded
     */
    public function getRecommended($networkLGF,
        $completedSections, $conditionValues, $testGrades,
        $nodeCosts, $nodePairCosts)
    {
        return $this->runCommand([
            'action' => 'recommend',
            'recType' => 'path',
            'network' => $networkLGF,
            'sections' => $completedSections,
            'conditions' => $conditionValues,
            'testGrades' => $testGrades,
            'nodeCosts' => $nodeCosts,
            'nodePairCosts' => $nodePairCosts
        ]);
    }


    /**
     * Create a learning net with unit nodes corresponding to the given section
     * ids.
     *
     * @param int[] $sectionIds section ids for units nodes of the new learning net
     * @return array array with a bool indicating whether the command succeeded
     * under the key 'succeeded' and error messages under the key 'message' if
     * the command failed or a network under the key 'message' if the command
     * succeeded
     */
    public function createNetwork($sectionIds)
    {
        return $this->runCommand([
            'action' => 'create',
            'sections' => $sectionIds
        ]);
    }
}
