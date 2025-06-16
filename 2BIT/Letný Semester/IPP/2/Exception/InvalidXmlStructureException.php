<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when XML structure is invalid
 */
class InvalidXmlStructureException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Invalid XML structure", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::INVALID_SOURCE_STRUCTURE_ERROR, $previous, false);
    }
}