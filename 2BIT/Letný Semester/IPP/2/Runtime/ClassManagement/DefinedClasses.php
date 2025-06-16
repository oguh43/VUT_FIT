<?php

namespace IPP\Student\Runtime\ClassManagement;

use IPP\Student\AST\Node\MethodNode;
use IPP\Student\Exception\DoesNotUnderstandException;

/**
 * Registry of all classes in the runtime
 */
class DefinedClasses
{
    /** @var array<string, ClassInfo> */
    private array $classes = [];

    /**
     * Register a class
     *
     * @param ClassInfo $classInfo Class information
     * @return void
     */
    public function register(ClassInfo $classInfo): void
    {
        $this->classes[$classInfo->getName()] = $classInfo;
    }

    /**
     * Get class information
     *
     * @param string|null $className Class name
     * @return ClassInfo|null Class information or null if not found
     */
    public function get(?string $className): ?ClassInfo
    {
        return $this->classes[$className] ?? null;
    }

    /**
     * Check if a class exists
     *
     * @param string $className Class name
     * @return bool True if the class exists
     */
    public function has(string $className): bool
    {
        return isset($this->classes[$className]);
    }

    /**
     * Find a method in a class or its ancestors
     *
     * @param string $className Class name
     * @param string $selector Method selector
     * @return MethodNode|null Method node or null if not found
     */
    public function resolveUserMethod(string $className, string $selector): ?MethodNode
    {
        $current = $this->get($className);
        
        while ($current !== null) {
            if ($current instanceof UserClassInfo && $current->hasMethod($selector)) {
                return $current->getMethodNode($selector);
            }

            $parentName = $current->getParentName();
            if ($parentName === null) {
                break;
            }

            $current = $this->get($parentName);
        }

        return null;
    }

    /**
     * Check if a class or any of its ancestors has a method
     *
     * @param string $className Class name
     * @param string $selector Method selector
     * @return boolean True if the method exists
     */
    public function hasMethod(string $className, string $selector): bool
    {
        $class = $this->get($className);
        
        while ($class !== null) {
            if ($class->hasMethod($selector)) {
                return true;
            }
            
            $parentName = $class->getParentName();
            $class = $parentName ? $this->get($parentName) : null;
        }

        return false;
    }

    /**
     * Find the nearest builtin class in the inheritance chain
     *
     * @param string $className Class name
     * @return string|null Builtin class name or null if not found
     */
    public function findBuiltinAncestor(string $className): ?string
    {
        $current = $this->get($className);
        
        while ($current !== null) {
            if ($current instanceof BuiltinClassInfo) {
                return $current->getName();
            }

            $parentName = $current->getParentName();
            $current = $this->get($parentName);
        }

        return null;
    }

    /**
     * Check if a class is a subclass of another
     *
     * @param string $className Potential subclass
     * @param string $potentialAncestorName Potential ancestor
     * @return bool True if className is a subclass of potentialAncestorName
     */
    public function isSubclassOf(string $className, string $potentialAncestorName): bool
    {
        if ($className === $potentialAncestorName) {
            return true;
        }

        $current = $this->get($className);
        
        while ($current !== null) {
            $parentName = $current->getParentName();
            
            if ($parentName === $potentialAncestorName) {
                return true;
            }
            
            $current = $this->get($parentName);
        }

        return false;
    }

    /**
     * Check if two classes are compatible for from: operation
     * Classes are compatible if one is a subclass of the other
     *
     * @param string $target Target class
     * @param string $source Source class
     * @return bool True if classes are compatible
     */
    public function areCompatibleClasses(string $target, string $source): bool
    {
        return $this->isSubclassOf($target, $source) || $this->isSubclassOf($source, $target);
    }

    /**
     * Get all registered classes
     *
     * @return array<string, ClassInfo> All classes
     */
    public function getAllClasses(): array
    {
        return $this->classes;
    }
}
