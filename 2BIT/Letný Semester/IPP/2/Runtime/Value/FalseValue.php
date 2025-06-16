<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 object False
 */
class FalseValue extends BooleanValue
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
        return 'False';
    }

    /**
     * Get raw value
     *
     * @return bool
     */
    public function getValue(): bool
    {
        return false;
    }

    /**
     * Implement not operation
     *
     * @return BooleanValue
     */
    protected function not(): BooleanValue
    {
        return self::getTrue();
    }

    /**
     * Implement and: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return BooleanValue
     */
    protected function and(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): BooleanValue {
        // Important: For false, and: just returns false without evaluating the block
        // This is critical: we don't validate the block for False since we never execute it
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("and: expects exactly one argument");
        }
        
        return self::getFalse();
    }

    /**
     * Implement or: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    protected function or(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        if (count($args) !== 1 || !($args[0] instanceof BlockValue)) {
            throw new DoesNotUnderstandException("or: expects a Block argument");
        }

        // For false, evaluate the argument
        return $args[0]->send('value', [], $definedClasses, $writer, $input);
    }

    /**
     * Implement ifTrue:ifFalse: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    protected function ifTrueIfFalse(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        if (count($args) !== 2) {
            throw new DoesNotUnderstandException("ifTrue:ifFalse: expects exactly two arguments");
        }

        // For false, evaluate the second block
        //if (!$args[1] instanceof BlockValue){throw new InvalidArgValueException("ifTrue:ifFalse: expects Block as first argument");}
        return $args[1]->send('value', [], $definedClasses, $writer, $input);
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
        return new StringValue('false', $definedClasses, $writer, $input);
    }
}