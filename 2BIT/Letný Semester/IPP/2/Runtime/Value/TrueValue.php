<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 object True
 */
class TrueValue extends BooleanValue
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
        return 'True';
    }

    /**
     * Get raw value
     *
     * @return bool
     */
    public function getValue(): bool
    {
        return true;
    }

    /**
     * Implement not operation
     *
     * @return BooleanValue
     */
    protected function not(): BooleanValue
    {
        return self::getFalse();
    }

    /**
     * Implement and: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    protected function and(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("and: expects exactly one argument");
        }
        
        if (!($args[0] instanceof BlockValue)) {
            throw new DoesNotUnderstandException("and: expects a Block argument");
        }

        // For true, and: evaluates the block
        return $args[0]->send('value', [], $definedClasses, $writer, $input);
    }

    /**
     * Implement or: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return BooleanValue
     */
    protected function or(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): BooleanValue {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("or: expects exactly one argument");
        }
        
        // If true, or: just returns true without evaluating the block
        return self::getTrue();
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

        // For true, evaluate the first block
        
        return $args[0]->send('value', [], $definedClasses, $writer, $input);
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
        return new StringValue('true', $definedClasses, $writer, $input);
    }
}