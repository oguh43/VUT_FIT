<?php

namespace IPP\Student\AST\Node;

use IPP\Core\Exception\XMLException;
use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Represents the root program node in the AST
 */
class ProgramNode extends AbstractNode
{
    private string $language;
    private ?string $description;
    /** @var array<ClassNode> */
    private array $classes = [];

    /**
     * Constructor
     *
     * @param \DOMDocument $dom XML DOM document
     */
    public function __construct(\DOMDocument $dom)
    {
        $program = $dom->getElementsByTagName('program')->item(0);

        // Node program doesn't exist
        if (!$program) {
            throw new XMLException("Node program doesn't exist");
        }

        $this->language = $program->getAttribute('language');
        $description = $program->getAttribute('description');
        $this->description = $description !== '' ? $description : null;

        foreach ($program->childNodes as $child) {
            if ($child instanceof \DOMElement && $child->tagName === 'class') {
                $this->classes[] = new ClassNode($child);
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
        return $visitor->visitProgram($this);
    }

    /**
     * Get the language
     *
     * @return string Language identifier
     */
    public function getLanguage(): string
    {
        return $this->language;
    }

    /**
     * Get the description
     *
     * @return string|null Program description
     */
    public function getDescription(): ?string
    {
        return $this->description;
    }

    /**
     * Get all classes
     *
     * @return array<ClassNode> All classes
     */
    public function getClasses(): array
    {
        return $this->classes;
    }

    /**
     * Get a class by name
     *
     * @param string $name Class name
     * @return ClassNode|null Class node or null if not found
     */
    public function getClass(string $name): ?ClassNode
    {
        foreach ($this->classes as $class) {
            if ($class->getName() === $name) {
                return $class;
            }
        }
        return null;
    }
}
