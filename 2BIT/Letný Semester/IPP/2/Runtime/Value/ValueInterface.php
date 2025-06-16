<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Common interface for all SOL25 value objects
 */
interface ValueInterface
{
    /**
     * Handle message send to this value
     *
     * @param string $selector Message selector
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return mixed Result of the message
     */
    public function send(
        string $selector,
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed;

    /**
     * Get the class name of this value
     *
     * @return string Class name
     */
    public function getClassName(): string;

    /**
     * Get the raw PHP value
     *
     * @return mixed Raw value
     */
    public function getValue(): mixed;

    /**
     * Set instance variable
     *
     * @param string $name Variable name
     * @param mixed $value Variable value
     * @return void
     */
    public function setInstanceVar(string $name, mixed $value): void;

    /**
     * Get instance variable
     *
     * @param string $name Variable name
     * @return mixed Variable value or null if not found
     */
    public function getInstanceVar(string $name): mixed;
}
