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

    /**
     * The singleton instance.
     *
     * @access  private
     * @var     NetworkCalculations
     */
    private static $instance = null;

    private static $pluginPath = "";

    /**
     * @access private
     *
     * @return void
     */
    private function __construct()
    { }

    /**
     * This method returns the singleton instance of this class.
     *
     * @param $path string The plugin path to use for finding the executable.
     * @return NetworkCalculations the singleton instance
     */
    static public function getInstance($path)
    {
        self::$pluginPath = $path;

        if (is_null(self::$instance)) {
            self::$instance = new NetworkCalculations();
        }
        return self::$instance;
    }

    private function buildCommand($args)
    {
        $command = self::$pluginPath . self::EXE_PATH;
        foreach ($args as $arg) {
            $command .= ' "' . $arg . '"';
        }
        return $command;
    }

    private function runCommand($args)
    {
        $output = array();
        $returnVal = null;

        exec($this->buildCommand($args), $output, $returnVal);

        return array(
            'output' => join("\n", $output),
            'returnValue' => $returnVal
        );
    }

    public function checkNetwork($networkLGF)
    {
        return $this->runCommand(array(
            '-check',
            '-network', $networkLGF)
        )['output'];
    }

    public function getActives($networkLGF, $completedSections)
    {
        return $this->runCommand(array(
            '-active',
            '-network', $networkLGF,
            '-sections', $completedSections)
        )['output'];
    }

    public function createEmptyNetwork($sectionIds)
    {
        return $this->runCommand(array(
            '-create',
            '-sections', $sectionIds)
        )['output'];
    }
}
