<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents a method definition in the AST
 */
class MethodNode extends AbstractNode
{
    private string $selector;
    private BlockNode $block;

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->selector = $element->getAttribute('selector');

        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement && $child->tagName === 'block') {
                $this->block = new BlockNode($child);
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
        return $visitor->visitMethod($this);
    }

    /**
     * Get method selector
     *
     * @return string Method selector
     */
    public function getSelector(): string
    {
        return $this->selector;
    }

    /**
     * Get method block
     *
     * @return BlockNode Method block
     */
    public function getBlock(): BlockNode
    {
        return $this->block;
    }
}
