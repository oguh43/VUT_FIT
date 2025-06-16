<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents a class definition in the AST
 */
class ClassNode extends AbstractNode
{
    private string $name;
    private string $parent;
    /** @var array<MethodNode> */
    private array $methods = [];

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->name = $element->getAttribute('name');
        $this->parent = $element->getAttribute('parent');

        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement && $child->tagName === 'method') {
                $this->methods[] = new MethodNode($child);
            }
        }
    }

    /**
     * Accept a visitor
     *
     * @param NodeVisitor $visitor The visitor to accept
     * @return mixed The result of the visit
     */
    public function accept(NodeVisitor $visitor): mixed
    {
        return $visitor->visitClass($this);
    }

    /**
     * Get class name
     *
     * @return string Class name
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Get parent class name
     *
     * @return string Parent class name
     */
    public function getParent(): string
    {
        return $this->parent;
    }

    /**
     * Get all methods
     *
     * @return array<MethodNode> All methods
     */
    public function getMethods(): array
    {
        return $this->methods;
    }

    /**
     * Get a method by name
     *
     * @param string $name Method name
     * @return MethodNode|null Method node or null if not found
     */
    public function getMethod(string $name): ?MethodNode
    {
        foreach ($this->methods as $method) {
            if ($method->getSelector() === $name) {
                return $method;
            }
        }
        return null;
    }
}
