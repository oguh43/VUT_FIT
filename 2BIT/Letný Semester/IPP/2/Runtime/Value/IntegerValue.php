<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 Integer object
 */
class IntegerValue extends AbstractValue
{
    /**
     * Constructor
     *
     * @param int $value Raw integer value
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     */
    public function __construct(
        private int $value,
        private DefinedClasses $definedClasses,
        private StreamWriter $writer,
        private InputReader $input
    ) {
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return 'Integer';
    }

    /**
     * Get raw value
     *
     * @return int
     */
    public function getValue(): int
    {
        return $this->value;
    }

    /**
     * Handle Integer-specific messages
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
            'greaterThan:' => $this->greaterThan($args),
            'plus:' => $this->plus($args),
            'minus:' => $this->minus($args),
            'multiplyBy:' => $this->multiplyBy($args),
            'divBy:' => $this->divBy($args),
            'asInteger' => $this,
            'timesRepeat:' => $this->timesRepeat($args, $definedClasses, $writer, $input),
            'isNumber' => BooleanValue::getTrue(),
            default => parent::handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Override equalTo: for integers
     *
     * @param array<mixed> $args
     * @return BooleanValue
     */
    protected function equalTo(array $args): BooleanValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("equalTo: expects exactly one argument");
        }
        
        try {
            $otherValue = $this->unwrapInteger($args[0], 'equalTo:');
            return BooleanValue::fromBool($this->value === $otherValue);
        } catch (InvalidArgValueException|DoesNotUnderstandException) {
            return BooleanValue::getFalse();
        }
    }

    /**
     * Implement greaterThan: operation
     *
     * @param array<mixed> $args
     * @return BooleanValue
     */
    protected function greaterThan(array $args): BooleanValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("greaterThan: expects exactly one argument");
        }

        $otherValue = $this->unwrapInteger($args[0], 'greaterThan:');
        
        // Fix: Ensure proper evaluation of greater than comparison
        return $this->value > $otherValue ? BooleanValue::getTrue() : BooleanValue::getFalse();
    }

    /**
     * Implement plus: operation
     *
     * @param array<mixed> $args
     * @return IntegerValue
     */
    protected function plus(array $args): IntegerValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("plus: expects exactly one argument");
        }

        $otherValue = $this->unwrapInteger($args[0], 'plus:');
        return new IntegerValue(
            $this->value + $otherValue,
            $this->definedClasses,
            $this->writer,
            $this->input
        );
    }

    /**
     * Implement minus: operation
     *
     * @param array<mixed> $args
     * @return IntegerValue
     */
    protected function minus(array $args): IntegerValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("minus: expects exactly one argument");
        }

        $otherValue = $this->unwrapInteger($args[0], 'minus:');
        return new IntegerValue(
            $this->value - $otherValue,
            $this->definedClasses,
            $this->writer,
            $this->input
        );
    }

    /**
     * Implement multiplyBy: operation
     *
     * @param array<mixed> $args
     * @return IntegerValue
     */
    protected function multiplyBy(array $args): IntegerValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("multiplyBy: expects exactly one argument");
        }

        $otherValue = $this->unwrapInteger($args[0], 'multiplyBy:');
        return new IntegerValue(
            $this->value * $otherValue,
            $this->definedClasses,
            $this->writer,
            $this->input
        );
    }

    /**
     * Implement divBy: operation
     *
     * @param array<mixed> $args
     * @return IntegerValue
     */
    protected function divBy(array $args): IntegerValue
    {
        if (count($args) !== 1) {
            throw new DoesNotUnderstandException("divBy: expects exactly one argument");
        }

        $otherValue = $this->unwrapInteger($args[0], 'divBy:');

        if ($otherValue === 0) {
            throw new InvalidArgValueException("Division by zero");
        }

        return new IntegerValue(
            intdiv($this->value, $otherValue),
            $this->definedClasses,
            $this->writer,
            $this->input
        );
    }

    /**
     * Implement timesRepeat: operation
     *
     * @param array<mixed> $args
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return NilValue
     */
    protected function timesRepeat(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): NilValue {
        if (count($args) !== 1 || !($args[0] instanceof BlockValue)) {
            throw new DoesNotUnderstandException("timesRepeat: expects a Block argument");
        }

        $block = $args[0];

        // Only execute if value is positive
        if ($this->value > 0) {
            for ($i = 1; $i <= $this->value; $i++) {
                $block->send(
                    'value:',
                    [new IntegerValue($i, $definedClasses, $writer, $input)],
                    $definedClasses,
                    $writer,
                    $input
                );
            }
        }

        return NilValue::getInstance();
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
        return new StringValue((string)$this->value, $definedClasses, $writer, $input);
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
            throw new DoesNotUnderstandException("Argument to {$operation} must be an Integer");
        }

        return $value;
    }
}