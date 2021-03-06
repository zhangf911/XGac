#include "GuiSolidLabelElementRenderer.h"
#include "CairoHelpers.h"

using namespace vl::collections;
using namespace vl::presentation::elements::text;
namespace vl
{
	namespace presentation
	{
		namespace elements_x11cairo
		{
			GuiSolidLabelElementRenderer::GuiSolidLabelElementRenderer()
				: minSize(1, 1), cairoContext(NULL), pangoFontDesc(NULL), attrList(NULL), layout(NULL)
			{
			}

			void GuiSolidLabelElementRenderer::InitializeInternal()
			{
				pangoFontDesc = pango_font_description_new();
				OnElementStateChanged();
			}

			void GuiSolidLabelElementRenderer::FinalizeInternal()
			{
				pango_font_description_free(pangoFontDesc);
				if(attrList)
					pango_attr_list_unref(attrList);
			}

			void GuiSolidLabelElementRenderer::Render(Rect bounds)
			{
				if(cairoContext)
				{
					cairo_save(cairoContext);
					Color color = element->GetColor();
					FontProperties font = element->GetFont();

					cairo_set_source_rgba(cairoContext, 
							1.0 * color.r / 255, 
							1.0 * color.g / 255, 
							1.0 * color.b / 255,
							1.0 * color.a / 255
							);

					if(element->GetWrapLine()) pango_layout_set_width(layout, bounds.Width() * PANGO_SCALE);
					pango_cairo_update_layout(cairoContext, layout);
					int layoutWidth, layoutHeight;
					int plotX1, plotY1;

					pango_layout_get_pixel_size( layout, &layoutWidth, &layoutHeight);
					switch(element->GetHorizontalAlignment())
					{
					case Alignment::Left:
						plotX1 = bounds.x1;
						break;
					case Alignment::Center:
						plotX1 = bounds.x1 + (bounds.Width() - layoutWidth) / 2;
						break;
					case Alignment::Right:
						plotX1 = bounds.x1 + (bounds.Width() - layoutWidth);
						break;
					}

					switch(element->GetVerticalAlignment())
					{
					case Alignment::Top:
						plotY1 = bounds.y1;
						break;
					case Alignment::Center:
						plotY1 = bounds.y1 + (bounds.Height() - layoutHeight) / 2;
						break;
					case Alignment::Bottom:
						plotY1 = bounds.y1 + (bounds.Height() - layoutHeight);
						break;
					}

					cairo_move_to(cairoContext, plotX1, plotY1);

					pango_cairo_layout_path(cairoContext, layout);
					cairo_fill(cairoContext);

					cairo_restore(cairoContext);
				}
			}

			void GuiSolidLabelElementRenderer::OnElementStateChanged()
			{
				FontProperties font = element->GetFont();
				Color color = element->GetColor();
				int layoutWidth, layoutHeight;
				
				AString family = wtoa(font.fontFamily);
				pango_font_description_set_family(pangoFontDesc, family.Buffer());
				pango_font_description_set_absolute_size(pangoFontDesc, font.size * PANGO_SCALE);

				if(font.italic) pango_font_description_set_style(pangoFontDesc, PANGO_STYLE_ITALIC);
				else pango_font_description_set_style(pangoFontDesc, PANGO_STYLE_NORMAL);

				if(attrList)
				{
					pango_attr_list_unref(attrList);
				}

				attrList = pango_attr_list_new();
				
				pango_attr_list_insert(attrList, pango_attr_underline_new(
							font.underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE)
					);

				pango_attr_list_insert(attrList, pango_attr_strikethrough_new (
							font.strikeline ? TRUE : FALSE
							)
						);

				pango_attr_list_insert(attrList, pango_attr_weight_new (
							font.bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_MEDIUM
							)
						);

				if(layout)
				{
					g_object_unref(layout);
					layout = NULL;
				}

				if(cairoContext)
				{
					layout = pango_cairo_create_layout(cairoContext);

					WString wtext = (font.fontFamily == L"Webdings") ? helpers::WebdingsMap(element->GetText()) : element->GetText();
					AString text = wtoa(wtext);

					pango_layout_set_font_description(layout, pangoFontDesc);
					pango_layout_set_attributes(layout, attrList);
					pango_layout_set_text(layout, text.Buffer(), text.Length());
					pango_layout_set_alignment(layout, 
							element->GetHorizontalAlignment() == Alignment::Left ? PANGO_ALIGN_LEFT :
							element->GetHorizontalAlignment() == Alignment::Center ? PANGO_ALIGN_CENTER :
							element->GetHorizontalAlignment() == Alignment::Right ? PANGO_ALIGN_RIGHT :
							PANGO_ALIGN_LEFT
							);


					pango_cairo_update_layout(cairoContext, layout);


					pango_layout_get_pixel_size( layout, &layoutWidth, &layoutHeight);

					minSize.x = layoutWidth;
					minSize.y = layoutHeight;
				}
			}

			void GuiSolidLabelElementRenderer::RenderTargetChangedInternal(IX11CairoRenderTarget* oldRT, IX11CairoRenderTarget* newRT)
			{
				if(newRT)
				{
					cairoContext = newRT->GetCairoContext();
					OnElementStateChanged();
				}
				else cairoContext = NULL;
			}
		}
	}
}
