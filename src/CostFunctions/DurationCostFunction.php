<?php

namespace LearningNet\CostFunctions;

use Mooc\DB\Block;
use Mooc\DB\Field;

/**
 * Estimates the time it takes to complete Courseware sections using video/audio
 * lengths and the word count of text content.
 *
 * @author  <milsen@uos.de>
 */
class DurationCostFunction extends NodeCostFunction
{
    /**
     * Estimates the time it takes to read the given text.
     *
     * @param string $text
     * @param int $wordsPerMinute
     * @return float estimated time in minutes to read the text
     */
    private function timeToRead($text, $wordsPerMinute = 100) {
        return str_word_count($text) / $wordsPerMinute;
    }

    /**
     * @param int $section id of the section for which to calculate its duration
     * @return float approximate time in min how long it takes to complete the
     * given section
     */
    public function calculate($section) {
        $duration = 0;

        $blockModel = new Block($section);
        $blocks = $blockModel->getContentChildren();

        foreach ($blocks as $block) {
            $id = $block->id;
            switch ($block->type) {
                // text-based blocks
                case 'TypewriterBlock':
                    $data = $this->getData($id, 'typewriter_json');
                    $duration += $this->timeToRead($data['content']);
                    break;
                case 'HtmlBlock':
                    $data = $this->getData($id, 'content');
                    $duration += $this->timeToRead(strip_tags($data));
                    break;
                case 'KeyPointBlock':
                    $data = $this->getData($id, 'keypoint_content');
                    $duration += $this->timeToRead($data);
                    break;
                case 'CodeBlock':
                    $data = $this->getData($id, 'code_content');
                    // CodeBlocks are more difficult to understand,
                    // assume a reading speed of 50 words per minute.
                    $duration += $this->timeToRead($data, 50);
                    break;
                case 'DialogCardsBlock':
                    $data = $this->getData($id, 'dialogcards_content');
                    $content = "";
                    foreach ($data as $card) {
                        $content .=
                            $card['front_text'] . " " .
                            $card['back_text'] . " ";
                    }
                    $duration += $this->timeToRead($content);
                    break;
                case 'PdfBlock':
                    $file = $this->getData($id, 'pdf_file');
                    // TODO
                    break;

                // pictures
                case 'CanvasBlock':
                    $duration += 5;
                    break;
                case 'ChartBlock':
                    $numDataSets = count($this->getData($id, 'chart_content'));
                    $duration += 2 * $numDataSets;
                    break;
                case 'GalleryBlock':
                    $numPics = count($this->getData($id, 'gallery_file_ids'));
                    $duration += 2 * $numPics;
                    break;
                case 'BeforeAfterBlock':
                    $duration += 4;
                    break;

                // test
                case 'TestBlock':
                    $data = $this->getData($id, 'assignment_id');
                    $db = \DBManager::get();
                    $stmt = $db->prepare("SELECT COUNT(*) FROM vips_exercise_ref "
                        . "WHERE test_id = :id");
                    $stmt->execute(['id' => $data]);
                    $numExercises = $stmt->fetchOne()['COUNT(*)'];
                    $duration += 4 * $numExercises;
                    break;

                // TODO add durations for the following block types
                // audio
                case 'AudioBlock':
                    break;
                case 'AudioGalleryBlock':
                    break;

                // video
                case 'InteractiveVideoBlock':
                    break;
                case 'OpenCastBlock':
                    break;
                case 'VideoBlock':
                    break;

                // ordering/grouping:
                case 'AssortBlock':
                    break;
                case 'ScrollyBlock':
                    break;

                // evaluation
                case 'EvaluationBlock':
                    break;

                // discussion
                case 'BlubberBlock':
                    break;
                case 'DiscussionBlock':
                    break;
                case 'PostBlock':
                    break;
                case 'ForumBlock':
                    break;

                // external content
                case 'LinkBlock':
                    break;
                case 'IFrameBlock':
                    break;
                case 'EmbedBlock':
                    break;

                // student has to acknowledge them but not do much
                case 'SearchBlock':
                case 'ConfirmBlock':
                case 'DateBlock':
                case 'DownloadBlock':
                    $duration += 0.5;
                    break;

                default:
                    break;
            }
        }

        return $duration;
    }
}
