<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when Main class or run method is missing
 */
class NoMainRunException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "No class Main with method run", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::PARSE_MAIN_ERROR, $previous, false);
    }
}