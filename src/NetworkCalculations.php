<?php

namespace LearningNet;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class NetworkCalculations
{
    const EXE_PATH = '/pathfinder/build/learningnet-pathfinder';

    private static $executablePath = "";

    public function __construct($pluginPath)
    {
        $this->executablePath = $pluginPath . self::EXE_PATH;
    }

    private function stringify($ids)
    {
        return join(" ", $ids);
    }

    private function buildCommand($args)
    {
        $command = $this->executablePath;
        foreach ($args as $arg) {
            $command .= ' "' . $arg . '"';
        }
        return $command;
    }

    private function runCommand($args)
    {
        $output = array();
        $returnVar = 0;

        exec($this->buildCommand($args), $output, $returnVar);

        return array(
            'message' => join("\n", $output),
            'succeeded' => $returnVar === 0
        );
    }

    public function checkNetwork($networkLGF)
    {
        return $this->runCommand(array(
            '-check',
            '-network', $networkLGF
        ));
    }

    public function getActives($networkLGF, $completedSections)
    {
        return $this->runCommand(array(
            '-active',
            '-network', $networkLGF,
            '-sections', $this->stringify($completedSections)
        ));
    }

    public function createNetwork($sectionIds)
    {
        return $this->runCommand(array(
            '-create',
            '-sections', $this->stringify($sectionIds)
        ));
    }
}
