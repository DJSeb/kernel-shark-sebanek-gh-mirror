/** TODO: Copyrgiht.. **/

#ifndef _KS_PLUGIN_STACKLOOK_HPP
#define _KS_PLUGIN_STACKLOOK_HPP

#include "KsPlotTools.hpp"

class StackTriangle : public KsPlot::Triangle {
protected:
    virtual void _doubleClick() const override;
};

#endif // !_KS_PLUGIN_STACKLOOK_HPP