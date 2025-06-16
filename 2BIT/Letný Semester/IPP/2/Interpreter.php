<?php

namespace IPP\Student;

use DOMXPath;
use DOMElement;
use DOMNodeList;
use IPP\Core\AbstractInterpreter;
use IPP\Core\Exception\XMLException;
use IPP\Core\StreamWriter;
use IPP\Core\ReturnCode;
use IPP\Student\AST\Node\ClassNode;
use IPP\Student\AST\Node\ProgramNode;
use IPP\Student\AST\Visitor\InterpreterVisitor;
use IPP\Student\Exception\InvalidXmlStructureException;
use IPP\Student\Exception\NoMainRunException;
use IPP\Student\Runtime\ClassManagement\BuiltinClassInfo;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;
use IPP\Student\Runtime\ClassManagement\UserClassInfo;
use IPP\Student\Runtime\SymbolTable;

/**
 * SOL25 Interpreter
 */
class Interpreter extends AbstractInterpreter
{
    /**
     * Execute the interpreter
     *
     * @return int Return code
     */
    public function execute(): int
    {
        try {
            // Parse XML and build AST
            $dom = $this->source->getDOMDocument();
            
            // Check if XML is well-formed
            if (is_null($dom->documentElement)) {
                throw new XMLException("The given XML code is not well-formed");
            }
            
            // Create program node
            $program = $this->buildAST($dom);
            
            // Validate XML structure
            $this->validateXmlStructure($dom);
            
            // Register classes
            $definedClasses = $this->registerClasses($program);
            
            // Execute program
            $symbolTable = new SymbolTable();
            $writer = new StreamWriter(STDOUT);
            $visitor = new InterpreterVisitor($symbolTable, $definedClasses, $writer, $this->input);
            $visitor->visitProgram($program);
            
            return ReturnCode::OK;
        } catch (\Exception $e) {
            throw $e;
        }
    }

    /**
     * Build the AST from the DOM
     *
     * @param \DOMDocument $dom Document
     * @return ProgramNode Program node
     */
    private function buildAST(\DOMDocument $dom): ProgramNode
    {
        $program = new ProgramNode($dom);
        
        // Verify SOL25 language
        if ($program->getLanguage() !== 'SOL25') {
            throw new XMLException("The language is not SOL25");
        }
        
        // Check for Main class with run method
        $mainClass = $program->getClass('Main');
        if (!$mainClass) {
            throw new NoMainRunException("Class Main not found");
        }
        
        $runMethod = $mainClass->getMethod('run');
        if (!$runMethod) {
            throw new NoMainRunException("No method run in the class Main found");
        }
        
        return $program;
    }

    /**
     * Validate XML structure for proper order attributes
     *
     * @param \DOMDocument $dom Document
     * @return void
     */
    private function validateXmlStructure(\DOMDocument $dom): void
    {
        $xpath = new DOMXPath($dom);
        
        // Check block elements
        $blocks = $xpath->query('//block');
        if (!$blocks instanceof DOMNodeList) {
            throw new \RuntimeException("XPath query failed");
        }
        
        foreach ($blocks as $block) {
            if ($block instanceof DOMElement) {
                $this->validateOrderedElements($block, 'assign');
                $this->validateOrderedElements($block, 'parameter');
            }
        }
        
        // Check send elements
        $sends = $xpath->query('//send');
        if (!$sends instanceof DOMNodeList) {
            throw new \RuntimeException("XPath query failed");
        }
        
        foreach ($sends as $send) {
            if ($send instanceof DOMElement) {
                $this->validateOrderedElements($send, 'arg');
            }
        }
    }

    /**
     * Validate that elements with order attribute are properly ordered
     *
     * @param DOMElement $parent Parent element
     * @param string $tagName Child tag name
     * @return void
     */
    private function validateOrderedElements(DOMElement $parent, string $tagName): void
    {
        $orders = [];
        
        foreach ($parent->childNodes as $child) {
            if ($child instanceof DOMElement && $child->tagName === $tagName) {
                $orders[] = (int)$child->getAttribute('order');
            }
        }
        
        $sortedOrders = $orders;
        sort($sortedOrders);
        
        if ($orders !== $sortedOrders) {
            throw new InvalidXmlStructureException(
                "Invalid <{$tagName}> order inside <{$parent->tagName}>. Found: " . implode(", ", $orders)
            );
        }
    }

    /**
     * Register all classes in the program
     *
     * @param ProgramNode $program Program node
     * @return DefinedClasses Defined classes
     */
    private function registerClasses(ProgramNode $program): DefinedClasses
    {
        $definedClasses = new DefinedClasses();
        
        // Register builtin classes
        $this->registerBuiltinClasses($definedClasses);
        
        // Register user classes
        foreach ($program->getClasses() as $classNode) {
            $this->registerUserClass($definedClasses, $classNode);
        }
        
        return $definedClasses;
    }

    /**
     * Register builtin classes
     *
     * @param DefinedClasses $definedClasses Defined classes
     * @return void
     */
    private function registerBuiltinClasses(DefinedClasses $definedClasses): void
    {
        // Object (root class)
        $definedClasses->register(new BuiltinClassInfo(
            'Object',
            ['identicalTo:', 'equalTo:', 'asString', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // Nil
        $definedClasses->register(new BuiltinClassInfo(
            'Nil',
            ['asString', 'identicalTo:', 'equalTo:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // Integer
        $definedClasses->register(new BuiltinClassInfo(
            'Integer',
            ['equalTo:', 'greaterThan:', 'plus:', 'minus:', 'multiplyBy:', 'divBy:', 
             'asString', 'asInteger', 'timesRepeat:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // String
        $definedClasses->register(new BuiltinClassInfo(
            'String',
            ['read', 'print', 'equalTo:', 'asString', 'asInteger', 'concatenateWith:',
             'startsWith:endsBefore:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // Block
        $definedClasses->register(new BuiltinClassInfo(
            'Block',
            ['whileTrue:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // True
        $definedClasses->register(new BuiltinClassInfo(
            'True',
            ['not', 'and:', 'or:', 'ifTrue:ifFalse:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
        
        // False
        $definedClasses->register(new BuiltinClassInfo(
            'False',
            ['not', 'and:', 'or:', 'ifTrue:ifFalse:', 'isNumber', 'isString', 'isBlock', 'isNil']
        ));
    }

    /**
     * Register a user class
     *
     * @param DefinedClasses $definedClasses Defined classes
     * @param ClassNode $classNode Class node
     * @return void
     */
    private function registerUserClass(DefinedClasses $definedClasses, ClassNode $classNode): void
    {
        $className = $classNode->getName();
        $parentName = $classNode->getParent();
        
        // Create methods array
        $methodNames = [];
        foreach ($classNode->getMethods() as $method) {
            $methodNames[] = $method->getSelector();
        }
        
        // Create class info
        $userClassInfo = new UserClassInfo($className, $parentName, $methodNames);
        
        // Register method implementations
        foreach ($classNode->getMethods() as $method) {
            $userClassInfo->addMethodNode($method->getSelector(), $method);
        }
        
        // Register class
        $definedClasses->register($userClassInfo);
    }
}
