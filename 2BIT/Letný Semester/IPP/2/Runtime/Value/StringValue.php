<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 String object
 */
class StringValue extends AbstractValue
{
    /**
     * Constructor
     *
     * @param string $value Raw string value
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     */
    public function __construct(
        private string $value,
        private DefinedClasses $definedClasses,
        private StreamWriter $writer,
        private InputReader $input
    ) {
        // Handle escape sequences
        $this->value = stripcslashes($this->value);
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return 'String';
    }

    /**
     * Get raw value
     *
     * @return string
     */
    public function getValue(): string
    {
        return $this->value;
    }

    /**
     * Handle String-specific messages
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
            'print' => $this->print($args),
            'asInteger' => $this->asInteger($args),
            'concatenateWith:' => $this->concatenateWith($args),
            'startsWith:endsBefore:' => $this->startsWithEndsBefore($args),
            'isString' => BooleanValue::getTrue(),
            default => parent::handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Override equalTo: for strings
     *
     * @param array<mixed> $args
     * @return BooleanValue
     */
    protected function equalTo(array $args): BooleanValue
    {
        try {
            $otherValue = $this->unwrapString($args[0], 'equalTo:');
            return BooleanValue::fromBool($this->value === $otherValue);
        } catch (InvalidArgValueException) {
            return BooleanValue::getFalse();
        }
    }

    /**
     * Implement print operation
     *
     * @param array<mixed> $args
     * @return StringValue
     */
    private function print(array $args): StringValue
    {
        if (count($args) !== 0) {
            throw new InvalidArgValueException("print expects no arguments");
        }

        $this->writer->writeString($this->value);
        return $this;
    }

    /**
     * Implement asInteger operation
     *
     * @param array<mixed> $args
     * @return IntegerValue|NilValue
     */
    private function asInteger(array $args): IntegerValue|NilValue
    {
        if (count($args) !== 0) {
            throw new InvalidArgValueException("asInteger expects no arguments");
        }

        // Remove whitespace
        $value = trim($this->value);

        // Create Integer instance if the value is valid integer
        if (filter_var($value, FILTER_VALIDATE_INT) !== false) {
            return new IntegerValue((int)$value, $this->definedClasses, $this->writer, $this->input);
        }

        return NilValue::getInstance();
    }

    /**
     * Implement concatenateWith: operation
     *
     * @param array<mixed> $args
     * @return StringValue|NilValue
     */
    private function concatenateWith(array $args): StringValue|NilValue
    {
        if (count($args) !== 1) {
            throw new InvalidArgValueException("concatenateWith: expects exactly one argument");
        }

        try {
            $otherValue = $this->unwrapString($args[0], 'concatenateWith:');
            return new StringValue(
                $this->value . $otherValue,
                $this->definedClasses,
                $this->writer,
                $this->input
            );
        } catch (InvalidArgValueException) {
            return NilValue::getInstance();
        }
    }

    /**
     * Implement startsWith:endsBefore: operation
     *
     * @param array<mixed> $args
     * @return StringValue|NilValue
     */
    private function startsWithEndsBefore(array $args): StringValue|NilValue
    {
        if (count($args) !== 2) {
            throw new InvalidArgValueException("startsWith:endsBefore: expects exactly two arguments");
        }

        try {
            $start = (int)$this->unwrapInteger($args[0], 'startsWith:endsBefore:');
            $end = (int)$this->unwrapInteger($args[1], 'startsWith:endsBefore:');

            if ($start <= 0 || $end <= 0) {
                return NilValue::getInstance();
            }

            if ($end - $start <= 0) {
                return new StringValue('', $this->definedClasses, $this->writer, $this->input);
            }

            $substr = substr($this->value, $start - 1, $end - $start);
            return new StringValue($substr, $this->definedClasses, $this->writer, $this->input);
        } catch (InvalidArgValueException) {
            return NilValue::getInstance();
        }
    }

    /**
     * Unwrap a string value
     *
     * @param mixed $value The value to unwrap
     * @param string $operation Operation name for error messages
     * @return string The raw string value
     */
    private function unwrapString(mixed $value, string $operation): string
    {
        if ($value instanceof ObjectValue) {
            $value = $value->getInstanceVar('__value');
        }

        if ($value instanceof StringValue) {
            $value = $value->getValue();
        }

        if (!is_string($value)) {
            throw new InvalidArgValueException("Argument to {$operation} must be a String");
        }

        return $value;
    }

    /**
     * Unwrap an integer value
     *
     * @param mixed $value The value to unwrap
     * @param string $operation Operation name for error messages
     * @return int The raw integer value
     */
    private function unwrapInteger(mixed $value, string $operation): int
    {
        if ($value instanceof ObjectValue) {
            $value = $value->getInstanceVar('__value');
        }

        if ($value instanceof IntegerValue) {
            $value = $value->getValue();
        }

        if (!is_int($value)) {
            throw new InvalidArgValueException("Argument to {$operation} must be an Integer");
        }

        return $value;
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
        return $this;
    }
}
