<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when a type error occurs
 */
class TypeErrorException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Type error", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::INTERPRET_TYPE_ERROR, $previous, false);
    }
}