//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    SceneNode.h
//	Purpose: Scene graph node. All scene node classes inherit from this
//          base class.
//
//============================================================================

#ifndef __SCENENODE_H
#define __SCENENODE_H

#include <vector>
#include <string>

/**
 * Scene graph node: base class
 */
class SceneNode {
public:
  /**
	 * Constructor. Set the reference count to 0.
	 */
  SceneNode() 
    : node_type(SCENE_BASE),
      reference_count(0) {
  } 

	/**
	 * Destructor
	 */
	virtual ~SceneNode() {
		Destroy();
	}

	/**
	 * Release this object from memory
	 */
	void Release() {
    // Decrement the reference count. Delete the object when reference 
    // count falls to 0
    reference_count--;
    if (reference_count <= 0)
      delete this; 
	}
	
	/**
	 * Draw the scene node and its children. The base class just draws the
   * children. Derived classes can use this (SceneNode::Draw()) to draw
   * all children without having to duplicate this code.
   * @param  scene_state  Current scene state
	 */
	virtual void Draw(SceneState& scene_state) {
		// Loop through the list and draw the children
    for (auto c : children) {
			c->Draw(scene_state);
    } 
	}
	
	/**
	 * Update the scene node and its children
   * @param  scene_state  Current scene state
	 */
	virtual void Update(SceneState& scene_state) {
		// Loop through the list and update the children
    for (auto c : children) {
			c->Update(scene_state);
    }
	}	
	
	/**
	 * Destroy all the children
	 */
	void Destroy() {
    for (auto c : children) {
      c->Release();
    }
    children.clear();
	}

	/**
	 * Add a child to this node. Increment the reference count of the child.
   * @param  node  Add a child node to this scene node.
	 */
	void AddChild(SceneNode* node) {
		children.push_back(node);
		node->reference_count++;
	}

  /**
	 * Get the type of scene node
   * @return  Returns the type of hte scene node.
	 */
	SceneNodeType GetNodeType() const {
		return node_type;
	}

  /**
   * Set the name for this scene node.
   * @param  name  Name for this scene node.
   */
  void SetName(const char* nm) {
		name = nm;
	}
	
  /**
   * Get the name for this scene node.
   * @return  Returns the name of this scene node.
   */
  const std::string& GetName() const {
    return name;
  }

protected:
	std::string             name;              
	SceneNodeType           node_type;
	int                     reference_count;
	std::vector<SceneNode*> children;
};

#endif
