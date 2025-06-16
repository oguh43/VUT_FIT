<?php

namespace IPP\Student\AST\Node;

use IPP\Core\Exception\XMLException;
use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents an expression in the AST
 */
class ExprNode extends AbstractNode
{
    private LiteralNode|VarNode|BlockNode|SendNode $node;

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement) {
                $tag = $child->tagName;
                $this->node = match ($tag) {
                    'literal' => new LiteralNode($child),
                    'var' => new VarNode($child),
                    'block' => new BlockNode($child),
                    'send' => new SendNode($child),
                    default => throw new XMLException("Unknown tag in expr: {$tag}")
                };
                break;
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
        return $visitor->visitExpr($this);
    }

    /**
     * Get the inner node
     *
     * @return LiteralNode|VarNode|BlockNode|SendNode The inner node
     */
    public function getNode(): LiteralNode|VarNode|BlockNode|SendNode
    {
        return $this->node;
    }
}
