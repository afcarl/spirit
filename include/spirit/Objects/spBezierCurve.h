#ifndef SP_BEZIERCURVE_H__
#define SP_BEZIERCURVE_H__

#include <spirit/Objects/spCommonObject.h>

class spBezierCurve : public spCommonObject {
 public:
  spBezierCurve();
  ~spBezierCurve();

  // SetPose here corresponds to the base of the curve
  void SetPose(const spPose& pose);
  const spPose& GetPose();

  void SetColor(const spColor& color);
  const spColor& GetColor();

  void SetControlPoints(const spBezierCtrlPoints& pts);
  void SetHermiteControlPoints(const spHermiteCtrlPoints& pts);
  const spBezierCtrlPoints& GetBezierControlPoints();

  void GetPoint(spPoint& point, double t);
  void GetPoints(spPoints& pts_vec, int num_mid_pts);


 private:
  spColor color_;
  spPose pose_;
  // Control points are in local coordinate
  spBezierCtrlPoints ctrl_pts_;
};

#endif  //  SP_BEZIERCURVE_H__