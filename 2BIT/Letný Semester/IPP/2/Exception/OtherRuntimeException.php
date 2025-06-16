<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown for other runtime errors (incompatible types, undefined variables, etc.)
 */
class OtherRuntimeException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Other runtime exception", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::INTERPRET_TYPE_ERROR, $previous, false);
    }
}
