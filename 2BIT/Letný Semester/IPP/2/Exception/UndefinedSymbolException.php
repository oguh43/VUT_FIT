<?php

namespace IPP\Student\Exception;

use IPP\Core\ReturnCode;
use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception thrown when an undefined symbol is referenced
 */
class UndefinedSymbolException extends IPPException
{
    /**
     * Constructor
     *
     * @param string $message Error message
     * @param Throwable|null $previous Previous exception
     */
    public function __construct(string $message = "Undefined symbol", ?Throwable $previous = null)
    {
        parent::__construct($message, ReturnCode::PARSE_UNDEF_ERROR, $previous, false);
    }
}