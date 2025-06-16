<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents a message send in the AST
 */
class SendNode extends AbstractNode
{
    private string $selector;
    private ExprNode $receiver;
    /** @var array<int, ExprNode> */
    private array $args = [];

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->selector = $element->getAttribute('selector');
        
        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement) {
                if ($child->tagName === 'expr') {
                    $this->receiver = new ExprNode($child);
                } elseif ($child->tagName === 'arg') {
                    foreach ($child->childNodes as $argChild) {
                        if ($argChild instanceof \DOMElement && $argChild->tagName === 'expr') {
                            $this->args[(int)$child->getAttribute('order')] = new ExprNode($argChild);
                        }
                    }
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
        return $visitor->visitSend($this);
    }

    /**
     * Get message selector
     *
     * @return string Message selector
     */
    public function getSelector(): string
    {
        return $this->selector;
    }

    /**
     * Get message receiver
     *
     * @return ExprNode Message receiver
     */
    public function getReceiver(): ExprNode
    {
        return $this->receiver;
    }

    /**
     * Get message arguments
     *
     * @return array<int, ExprNode> Message arguments (ordered by position)
     */
    public function getArgs(): array
    {
        return $this->args;
    }
}
