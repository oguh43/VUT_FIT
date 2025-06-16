<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when an argument value is invalid (e.g. divide by zero)
 */
class InvalidArgValueException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Invalid argument value", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::INTERPRET_VALUE_ERROR, $previous, false);
    }
}