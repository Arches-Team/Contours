// Palettes

#pragma once

 #include "color.h"

class GenericPalette {
 public:
   virtual Color GetColor(double) const;

};

class AnalyticPalette : public GenericPalette {
protected:
  int n = 0; //!< Palette identifier.
  bool r = false; //!< Reverse flag.
public:
  explicit AnalyticPalette(int = 0, bool = false);
  virtual Color GetColor(double) const;

public:
  static Color BrownGreyGreen(double);
  static Color GreenBrownGrey(double);
  static Color CoolWarm(double);
  static Color MatlabJet(double);
  static Color BlueGreen(double);
  static Color WhiteRed(double);
  static Color BlueGreyBrown(double);
  static Color GeologyGreenYellow(double);
  static Color GeologyGreenYellow2(double);
  static Color BrownGreen(double);
  static Color WhiteBlue(double);
  static Color WhiteBrown(double);
  static Color GreenOrange(double);

private:
  static Color Diverging(const Color&, const Color&, const Color&, double);
  double Reverse(double) const;
};

class Palette : public GenericPalette
{
protected:
  QVector<Color> c;  //!< %Array of colors.
  QVector<double> a; //!< Anchors.
  int type = 0;      //!< Type, used to speed-up queries.
public:
  Palette();
  Palette(const QVector<QColor>&);
  Palette(const QVector<Color>&);

  Color GetColor(double) const;
};

