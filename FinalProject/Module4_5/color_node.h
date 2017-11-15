//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    ColorNode.h
//	Purpose:	Simple presentation node that defines a color.
//
//============================================================================

#ifndef __COLORNODE_H
#define __COLORNODE_H

/**
 * Color presentation node.
 */
class ColorNode: public PresentationNode {
public:
  /**
   * Constructor.
   */
  ColorNode()
    : PresentationNode() {
  }

  /**
   * Constructor given a red, green, and blue value.
   */
  ColorNode(const Color4& c) {
    material_color = c;
  }

  /**
   * Draw this presentation node and its children
   */
  virtual void Draw(SceneState& scene_state) {
    // Set the current color and draw all children. Very simple lighting support
    glUniform3fv(scene_state.materialdiffuse_loc, 1, &material_color.r);
    SceneNode::Draw(scene_state);
  }

protected:
   Color4 material_color;
};

#endif