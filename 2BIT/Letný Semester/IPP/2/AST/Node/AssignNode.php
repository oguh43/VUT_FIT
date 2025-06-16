<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents an assignment statement in the AST
 */
class AssignNode extends AbstractNode
{
    private int $order;
    private VarNode $var;
    private ExprNode $expr;

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->order = (int)$element->getAttribute('order');

        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement) {
                if ($child->tagName === 'var') {
                    $this->var = new VarNode($child);
                } elseif ($child->tagName === 'expr') {
                    $this->expr = new ExprNode($child);
                }
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
        return $visitor->visitAssign($this);
    }

    /**
     * Get statement order
     *
     * @return int Statement order
     */
    public function getOrder(): int
    {
        return $this->order;
    }

    /**
     * Get variable node
     *
     * @return VarNode Variable node
     */
    public function getVar(): VarNode
    {
        return $this->var;
    }

    /**
     * Get expression node
     *
     * @return ExprNode Expression node
     */
    public function getExpr(): ExprNode
    {
        return $this->expr;
    }
}
