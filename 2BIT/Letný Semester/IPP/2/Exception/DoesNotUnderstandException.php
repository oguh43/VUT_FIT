<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when an object doesn't understand a message
 */
class DoesNotUnderstandException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Does not understand", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::INTERPRET_DNU_ERROR, $previous, false);
    }
}