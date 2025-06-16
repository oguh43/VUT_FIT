<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Abstract base class for all SOL25 value objects
 * Provides common functionality
 */
abstract class AbstractValue implements ValueInterface
{
    /** @var array<string, mixed> */
    protected array $instanceVars = [];

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
    ): mixed {
        // Handle setter pattern (variable:)
        if (str_ends_with($selector, ':') && count($args) === 1 && !$definedClasses->hasMethod($this->getClassName(), $selector)) {
            $varName = substr($selector, 0, -1);
            $this->instanceVars[$varName] = $args[0];
            return $this;
        }

        // Handle getter pattern (variable)
        if (array_key_exists($selector, $this->instanceVars)) {
            return $this->instanceVars[$selector];
        }

        // Handle standard object messages
        return match ($selector) {
            'identicalTo:' => $this->identicalTo($args),
            'equalTo:' => $this->equalTo($args),
            'asString' => $this->asString($definedClasses, $writer, $input),
            'isNumber' => BooleanValue::getFalse(),
            'isString' => BooleanValue::getFalse(),
            'isBlock' => BooleanValue::getFalse(),
            'isNil' => BooleanValue::getFalse(),
            default => $this->handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Handle class-specific messages
     * To be implemented by subclasses
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
        throw new DoesNotUnderstandException("Class {$this->getClassName()} doesn't understand message: {$selector}");
    }

    /**
     * Default implementation of identicalTo:
     *
     * @param array<mixed> $args Arguments (should be [object])
     * @return ValueInterface True or False object
     */
    protected function identicalTo(array $args): ValueInterface
    {
        return ($this === $args[0]) ? BooleanValue::getTrue() : BooleanValue::getFalse();
    }

    /**
     * Default implementation of equalTo:
     * By default, uses identicalTo:, but can be overridden
     *
     * @param array<mixed> $args Arguments (should be [object])
     * @return ValueInterface True or False object
     */
    protected function equalTo(array $args): ValueInterface
    {
        return $this->identicalTo($args);
    }

    /**
     * Default implementation of asString
     * Should be overridden by subclasses
     *
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return ValueInterface String value
     */
    protected function asString(
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): ValueInterface {
        return new StringValue('', $definedClasses, $writer, $input);
    }

    /**
     * Set instance variable
     *
     * @param string $name Variable name
     * @param mixed $value Variable value
     * @return void
     */
    public function setInstanceVar(string $name, mixed $value): void
    {
        $this->instanceVars[$name] = $value;
    }

    /**
     * Get instance variable
     *
     * @param string $name Variable name
     * @return mixed Variable value or null if not found
     */
    public function getInstanceVar(string $name): mixed
    {
        return $this->instanceVars[$name] ?? null;
    }

    /**
     * Get all instance variables
     *
     * @return array<string, mixed> All instance variables
     */
    public function getInstanceVars(): array
    {
        return $this->instanceVars;
    }
}