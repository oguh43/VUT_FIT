<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 Nil object
 */
class NilValue extends AbstractValue
{
    private static ?self $instance = null;

    /**
     * Private constructor (Singleton pattern)
     */
    private function __construct()
    {
    }

    /**
     * Get singleton instance
     *
     * @return self
     */
    public static function getInstance(): self
    {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return 'Nil';
    }

    /**
     * Get raw value
     *
     * @return mixed
     */
    public function getValue(): mixed
    {
        return null;
    }

    /**
     * Handle Nil-specific messages
     *
     * @param string $selector Message selector
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return mixed Result of the message
     */
    protected function handleSpecificMessage(
        string $selector,
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        return match ($selector) {
            'isNil' => BooleanValue::getTrue(),
            default => parent::handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Create string representation
     *
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return StringValue
     */
    protected function asString(
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): StringValue {
        return new StringValue('nil', $definedClasses, $writer, $input);
    }
}
