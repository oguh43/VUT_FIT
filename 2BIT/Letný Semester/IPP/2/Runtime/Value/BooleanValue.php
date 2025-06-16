<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Base class for boolean values (True and False)
 */
abstract class BooleanValue extends AbstractValue
{
    /**
     * Get a True value singleton
     *
     * @return TrueValue
     */
    public static function getTrue(): TrueValue
    {
        return TrueValue::getInstance();
    }

    /**
     * Get a False value singleton
     *
     * @return FalseValue
     */
    public static function getFalse(): FalseValue
    {
        return FalseValue::getInstance();
    }

    /**
     * Get boolean value from PHP bool
     *
     * @param bool $value
     * @return BooleanValue
     */
    public static function fromBool(bool $value): BooleanValue
    {
        return $value ? self::getTrue() : self::getFalse();
    }

    /**
     * Handle boolean-specific messages
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
            'not' => $this->not(),
            'and:' => $this->and($args, $definedClasses, $writer, $input),
            'or:' => $this->or($args, $definedClasses, $writer, $input),
            'ifTrue:ifFalse:' => $this->ifTrueIfFalse($args, $definedClasses, $writer, $input),
            default => parent::handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Implement 'not' operation
     *
     * @return BooleanValue
     */
    abstract protected function not(): BooleanValue;

    /**
     * Implement 'and:' operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    abstract protected function and(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed;

    /**
     * Implement 'or:' operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    abstract protected function or(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed;

    /**
     * Implement 'ifTrue:ifFalse:' operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return mixed
     */
    abstract protected function ifTrueIfFalse(
        array $args,
        DefinedClasses $definedClasses, 
        StreamWriter $writer,
        InputReader $input
    ): mixed;

    /**
     * Default implementation of equalTo:
     *
     * @param array<mixed> $args Arguments
     * @return ValueInterface True or False value
     */
    protected function equalTo(array $args): ValueInterface
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("equalTo: expects exactly one argument");
        }
        
        try {
            $otherValue = $this->unwrapBoolean($args[0], 'equalTo:');
            return self::fromBool($this->getValue() === $otherValue);
        } catch (DoesNotUnderstandException) {
            return self::getFalse();
        }
    }

    /**
     * Unwrap a boolean value
     *
     * @param mixed $value The value to unwrap
     * @param string $operation Operation name for error messages
     * @return bool The raw boolean value
     */
    protected function unwrapBoolean(mixed $value, string $operation): bool
    {
        if ($value instanceof ObjectValue) {
            $value = $value->getInstanceVar('__value');
        }
        if ($value instanceof BooleanValue){
            $result = $value->getValue();
            if (is_bool($result)) {
                return $result;
            }
        }
    
        throw new DoesNotUnderstandException("Argument to {$operation} must be a Boolean value");
    }
}