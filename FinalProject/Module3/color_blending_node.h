//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    color_blending_node.h
//	Purpose:Simple presentation node that defines a color and blending.
//
//============================================================================

#ifndef __COLORBLENDINGNODE_H
#define __COLORBLENDINGNODE_H

/**
 * Color presentation node with blending.
 */
class ColorBlendingNode: public PresentationNode {
public:
  /**
   * Constructor given a red, green, blue, and alpha value.
   * @param  c  Color including alpha.
   * @param  b  Blending enabled (true or false).
   */
  ColorBlendingNode(const Color4& c, const bool b) {
    color = c;
    blending = b;
  }

  /**
   * Draw this presentation node and its children
   */
  virtual void Draw(SceneState& scene_state) {
    // Enable blending 
    if (blending) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
    }
    
    // Set the current color and draw all children.
    glUniform4fv(scene_state.color_loc, 1, &color.r);
    SceneNode::Draw(scene_state);

    // Disable blending so it is not enabled except for children of this node
    if (blending) {
      glDisable(GL_BLEND);
    }
  }

protected:
  bool blending;
  Color4 color;
};

#endif