#include "descriptorgroup.h"

#include "context.h"
#include "conversions.h"

#include "vko/Device.h"
#include "vko/Assert.h"

gfx_vk::descriptorgroup::descriptorgroup(context& context, gfx::descriptorgroup_params const&)
    : m_context(context)
{
    assert(false); // TODO
}

gfx_vk::descriptorgroup::~descriptorgroup()
{

}
