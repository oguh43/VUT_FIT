<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\BuiltinClassInfo;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;

/**
 * Represents SOL25 Class object
 * Handles factory methods (constructors)
 */
class ClassValue implements ValueInterface
{
    /**
     * Constructor
     *
     * @param string $className Class name
     */
    public function __construct(private string $className)
    {
    }

    /**
     * Handle message send to this class
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
        return match ($selector) {
            'new' => $this->createNew($args, $definedClasses, $writer, $input),
            'from:' => $this->createFrom($args, $definedClasses, $writer, $input),
            'read' => $this->read($args, $definedClasses, $writer, $input),
            default => throw new DoesNotUnderstandException(
                "Class {$this->className} doesn't understand message: {$selector}"
            )
        };
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return 'Class';
    }

    /**
     * Get raw value
     *
     * @return string Class name
     */
    public function getValue(): string
    {
        return $this->className;
    }

    /**
     * Set instance variable
     * (Not applicable for ClassValue)
     *
     * @param string $name Variable name
     * @param mixed $value Variable value
     * @return void
     */
    public function setInstanceVar(string $name, mixed $value): void
    {
        // Class has no instance variables
    }

    /**
     * Get instance variable
     * (Not applicable for ClassValue)
     *
     * @param string $name Variable name
     * @return mixed Variable value
     */
    public function getInstanceVar(string $name): mixed
    {
        return null;
    }

    /**
     * Implement 'new' constructor
     *
     * @param array<mixed> $args Constructor arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return ValueInterface The new instance
     */
    private function createNew(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): ValueInterface {
        if (!empty($args)) {
            throw new InvalidArgValueException("new expects no arguments");
        }

        // Handle builtin classes
        $classInfo = $definedClasses->get($this->className);
        if ($classInfo instanceof BuiltinClassInfo) {
            return match ($this->className) {
                'Integer' => new IntegerValue(0, $definedClasses, $writer, $input),
                'String' => new StringValue('', $definedClasses, $writer, $input),
                'Nil' => NilValue::getInstance(),
                'True' => TrueValue::getInstance(),
                'False' => FalseValue::getInstance(),
                'Object', 'Block' => new ObjectValue($this->className),
                default => throw new InvalidArgValueException("Cannot instantiate unknown class: {$this->className}")
            };
        }

        // Create user-defined object
        return new ObjectValue($this->className);
    }

    /**
     * Implement 'from:' constructor
     *
     * @param array<mixed> $args Constructor arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return ValueInterface The new instance
     */
    private function createFrom(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): ValueInterface {
        if (count($args) !== 1) {
            throw new InvalidArgValueException("from: expects exactly one argument");
        }

        $sourceObj = $args[0];
        if (!$sourceObj instanceof ValueInterface){
            throw new InvalidArgValueException("Expected argument to be of type ValueInterface");
        }
        $sourceClass = $sourceObj->getClassName();

        // Check type compatibility
        if (!$definedClasses->areCompatibleClasses($this->className, $sourceClass)) {
            throw new InvalidArgValueException(
                "Class {$sourceClass} is not compatible with {$this->className}"
            );
        }

        // Handle builtin classes
        if ($definedClasses->get($this->className) instanceof BuiltinClassInfo) {
            switch ($this->className) {
                case 'Integer':
                    $value = $this->unwrapValue($sourceObj);
                    if (!is_int($value) && !is_null($value)) {
                        throw new InvalidArgValueException("Cannot convert to Integer");
                    }
                    return new IntegerValue($value ?? 0, $definedClasses, $writer, $input);

                case 'String':
                    $value = $this->unwrapValue($sourceObj);
                    if (!is_string($value) && !is_null($value)) {
                        throw new InvalidArgValueException("Cannot convert to String");
                    }
                    return new StringValue($value ?? '', $definedClasses, $writer, $input);

                case 'Nil':
                    return NilValue::getInstance();

                case 'True':
                    return TrueValue::getInstance();

                case 'False':
                    return FalseValue::getInstance();

                default:
                    // Handle Block separately
                    if ($this->className === 'Block' && $sourceObj instanceof BlockValue) {
                        $instance = new ObjectValue($this->className);
                        $instance->setInstanceVar('__block', $sourceObj);
                        return $instance;
                    }
            }
        }

        // Create user-defined object
        $instance = new ObjectValue($this->className);

        // Copy the source value
        $rawValue = $this->unwrapValue($sourceObj);
        if ($rawValue !== null) {
            $instance->setInstanceVar('__value', $rawValue);
        }

        // Copy other instance variables
        if (method_exists($sourceObj, 'getInstanceVars')) {
            foreach ($sourceObj->getInstanceVars() as $name => $value) {
                // Skip special internal variables
                if ($name !== '__value' && $name !== '__block') {
                    $instance->setInstanceVar($name, $value);
                }
            }
        }

        return $instance;
    }

    /**
     * Implement 'read' constructor (for String only)
     *
     * @param array<mixed> $args Constructor arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return ValueInterface The new instance
     */
    private function read(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): ValueInterface {
        if (!empty($args)) {
            throw new InvalidArgValueException("read expects no arguments");
        }

        if ($this->className !== 'String') {
            throw new DoesNotUnderstandException("Class {$this->className} doesn't understand message: read");
        }

        return new StringValue($input->readString() ?? '', $definedClasses, $writer, $input);
    }

    /**
     * Extract the raw value from an object
     *
     * @param ValueInterface $obj Source object
     * @return mixed The raw value or null
     */
    private function unwrapValue(ValueInterface $obj): mixed
    {
        if ($obj instanceof ObjectValue) {
            return $obj->getInstanceVar('__value');
        }

        
        return $obj->getValue();
        

    }
}
