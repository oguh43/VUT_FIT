<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents a variable reference in the AST
 */
class VarNode extends AbstractNode
{
    private string $name;

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->name = $element->getAttribute('name');
    }

    /**
     * Accept a visitor
     *
     * @param NodeVisitor $visitor The visitor to accept
     * @return mixed The result of the visit
     */
    public function accept(NodeVisitor $visitor): mixed
    {
        return $visitor->visitVar($this);
    }

    /**
     * Get variable name
     *
     * @return string Variable name
     */
    public function getName(): string
    {
        return $this->name;
    }
}
