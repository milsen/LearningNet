<?php

namespace LearningNet;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class NetworkCalculations
{
    const EXE_PATH = '/backend/build/learningnet-pathfinder';

    private static $executablePath = "";

    public function __construct($pluginPath)
    {
        $this->executablePath = $pluginPath . self::EXE_PATH;
    }

    private function buildCommand($arg)
    {
        return $this->executablePath . ' \'' . json_encode($arg) . '\' 2>&1';
    }

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

    public function checkNetwork($networkLGF)
    {
        return $this->runCommand([
            'action' => 'check',
            'network' => $networkLGF
        ]);
    }

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

    public function createNetwork($sectionIds)
    {
        return $this->runCommand([
            'action' => 'create',
            'sections' => $sectionIds
        ]);
    }
}
