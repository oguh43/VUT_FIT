<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;
use IPP\Student\Runtime\SymbolTable;

/**
 * Represents SOL25 user-defined objects
 */
class ObjectValue extends AbstractValue
{
    /**
     * Constructor
     *
     * @param string $className Class name (defaults to 'Object')
     */
    public function __construct(private string $className = 'Object')
    {
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return $this->className;
    }

    /**
     * Get raw value (always returns null for Objects)
     *
     * @return mixed
     */
    public function getValue(): mixed
    {
        return $this->getInstanceVar('__value') ?? null;
    }

    /**
     * Handle message send to this object
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
        // Try to find a user-defined method
        $method = $definedClasses->resolveUserMethod($this->className, $selector);
        if ($method) {
            // Create a new symbol table for method execution
            $symbols = new SymbolTable();
            $symbols->define('self', $this);

            // Bind parameters
            $methodBlock = $method->getBlock();
            foreach ($methodBlock->getParameters() as $i => $param) {
                if (is_string($param['name'])){
                    $symbols->define($param['name'], $args[$i]);
                }
            }

            // Execute the method block
            return $methodBlock->accept(new \IPP\Student\AST\Visitor\InterpreterVisitor(
                $symbols,
                $definedClasses,
                $writer,
                $input
            ));
        }

        // Handle setter pattern
        if (str_ends_with($selector, ':') && count($args) === 1 && !$definedClasses->hasMethod($this->className, $selector)) {
            $varName = substr($selector, 0, -1);
            $this->instanceVars[$varName] = $args[0];
            return $this;
        }

        // Handle getter pattern
        if (array_key_exists($selector, $this->instanceVars)) {
            return $this->instanceVars[$selector];
        }

        // Try to delegate to builtin parent if any
        $builtinParent = $definedClasses->findBuiltinAncestor($this->className);
        if ($builtinParent) {
            return $this->delegateToBuiltin($builtinParent, $selector, $args, $definedClasses, $writer, $input);
        }

        // Handle common Object messages
        return match ($selector) {
            'equalTo:' => $this->identicalTo($args),
            'identicalTo:' => $this->identicalTo($args),
            'asString' => new StringValue('', $definedClasses, $writer, $input),
            'isNumber' => BooleanValue::getFalse(),
            'isString' => BooleanValue::getFalse(),
            'isBlock' => BooleanValue::getFalse(),
            'isNil' => BooleanValue::getFalse(),
            default => throw new DoesNotUnderstandException(
                "Class {$this->className} doesn't understand message: {$selector}"
            )
        };
    }

    /**
     * Delegate message to a builtin parent class
     *
     * @param string $builtinParent Parent class name
     * @param string $selector Message selector
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return mixed Result of the message
     */
    private function delegateToBuiltin(
        string $builtinParent,
        string $selector,
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        // Delegate to specific builtin parent
        return match ($builtinParent) {
            'Nil' => NilValue::getInstance()->send($selector, $args, $definedClasses, $writer, $input),
            'Integer' => $this->createIntegerDelegate($definedClasses, $writer, $input)
                ->send($selector, $args, $definedClasses, $writer, $input),
            'String' => $this->createStringDelegate($definedClasses, $writer, $input)
                ->send($selector, $args, $definedClasses, $writer, $input),
            'Block' => $this->createBlockDelegate()
                ->send($selector, $args, $definedClasses, $writer, $input),
            'True' => TrueValue::getInstance()->send($selector, $args, $definedClasses, $writer, $input),
            'False' => FalseValue::getInstance()->send($selector, $args, $definedClasses, $writer, $input),
            default => throw new DoesNotUnderstandException(
                "Class {$this->className} doesn't understand message: {$selector}"
            )
        };
    }

    /**
     * Create an Integer delegate
     *
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return IntegerValue Integer delegate
     */
    private function createIntegerDelegate(
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): IntegerValue {
        $value = $this->getInstanceVar('__value');
        if (!is_int($value)) {
            throw new InvalidArgValueException("Expected integer value for {$this->className}");
        }
        return new IntegerValue($value, $definedClasses, $writer, $input);
    }

    /**
     * Create a String delegate
     *
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return StringValue String delegate
     */
    private function createStringDelegate(
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): StringValue {
        $value = $this->getInstanceVar('__value');
        if (!is_string($value)) {
            throw new InvalidArgValueException("Expected string value for {$this->className}");
        }
        return new StringValue($value, $definedClasses, $writer, $input);
    }

    /**
     * Create a Block delegate
     *
     * @return BlockValue Block delegate
     */
    private function createBlockDelegate(): BlockValue
    {
        $block = $this->getInstanceVar('__block');
        if (!$block instanceof BlockValue) {
            throw new InvalidArgValueException("Expected block value for {$this->className}");
        }
        return $block;
    }
}