<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;
use IPP\Student\Runtime\Value\BooleanValue;
use IPP\Student\Runtime\Value\ClassValue;
use IPP\Student\Runtime\Value\IntegerValue;
use IPP\Student\Runtime\Value\NilValue;
use IPP\Student\Runtime\Value\StringValue;

/**
 * Represents a literal in the AST
 */
class LiteralNode extends AbstractNode
{
    private string $class;
    private string $value;

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->class = $element->getAttribute('class');
        $this->value = $element->getAttribute('value');
    }

    /**
     * Accept a visitor
     *
     * @param NodeVisitor $visitor The visitor to accept
     * @return mixed The result of the visit
     */
    public function accept(NodeVisitor $visitor): mixed
    {
        return $visitor->visitLiteral($this);
    }

    /**
     * Get literal class
     *
     * @return string Literal class
     */
    public function getClass(): string
    {
        return $this->class;
    }

    /**
     * Get literal value
     *
     * @return string Literal value
     */
    public function getValue(): string
    {
        return $this->value;
    }

    /**
     * Helper to evaluate this literal to its runtime value
     * Note: This is used by the InterpreterVisitor
     *
     * @param \IPP\Core\StreamWriter $writer Output writer
     * @param \IPP\Core\Interface\InputReader $input Input reader
     * @param \IPP\Student\Runtime\ClassManagement\DefinedClasses $definedClasses Available classes
     * @return mixed The runtime value
     */
    public function evaluateToValue(
        \IPP\Core\StreamWriter $writer,
        \IPP\Core\Interface\InputReader $input,
        \IPP\Student\Runtime\ClassManagement\DefinedClasses $definedClasses
    ): mixed {
        return match ($this->class) {
            'String' => new StringValue($this->value, $definedClasses, $writer, $input),
            'Integer' => new IntegerValue((int)$this->value, $definedClasses, $writer, $input),
            'Nil' => NilValue::getInstance(),
            'True' => BooleanValue::getTrue(),
            'False' => BooleanValue::getFalse(),
            'class' => new ClassValue($this->value),
            default => throw new \IPP\Student\Exception\OtherRuntimeException("Unsupported literal class: {$this->class}")
        };
    }
}
