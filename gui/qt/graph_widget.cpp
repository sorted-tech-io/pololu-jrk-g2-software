#include "graph_widget.h"

#include <QString>

graph_widget::graph_widget(QWidget * parent)
{
  setup_ui();

  connect(max_y, SIGNAL(valueChanged(double)),
    this, SLOT(change_ranges()));

  connect(min_y, SIGNAL(valueChanged(double)),
    this, SLOT(change_ranges()));

  connect(domain, SIGNAL(valueChanged(int)),
    this, SLOT(change_ranges()));
}

// Changes options for the custom_plot when in preview mode.
void graph_widget::set_preview_mode(bool preview_mode)
{
  if (preview_mode)
  {
    custom_plot->setCursor(Qt::PointingHandCursor);
    custom_plot->setToolTip("Click to open graph window");
    custom_plot->axisRect()->setAutoMargins(QCP::msNone);
    custom_plot->axisRect()->setMargins(QMargins(5, 5, 5, 5));
    custom_plot->xAxis->setBasePen(QColor(Qt::white));
    custom_plot->yAxis->setBasePen(QColor(Qt::white));
  }
  else
  {
    custom_plot->setCursor(Qt::ArrowCursor);
    custom_plot->setToolTip("");
    custom_plot->axisRect()->setAutoMargins(QCP::msAll);
    custom_plot->xAxis->setBasePen(QColor(Qt::black));
    custom_plot->yAxis->setBasePen(QColor(Qt::black));
  }

  custom_plot->xAxis->setTicks(!preview_mode);
  custom_plot->yAxis->setTicks(!preview_mode);

  custom_plot->replot();
}

void graph_widget::set_paused(bool paused)
{
  if (paused != graph_paused)
  {
    graph_paused = paused;
    pause_run_button->setText(graph_paused ? "R&un" : "&Pause");
    custom_plot->replot();
  }
}

void graph_widget::clear_graphs()
{
  for (auto plot : all_plots)
  {
    plot->graph->data()->clear();
  }

  custom_plot->replot();
}

void graph_widget::plot_data(uint32_t time)
{
  for (auto plot : all_plots)
  {
    plot->graph->addData(time, plot->plot_value);
  }

  if (graph_paused)
    return;

  remove_data_to_scroll(time);
}

void graph_widget::setup_ui()
{
  pause_run_button = new QPushButton(this);
  pause_run_button->setObjectName("pause_run_button");
  pause_run_button->setText(tr("&Pause"));
  pause_run_button->setMinimumSize(pause_run_button->sizeHint());

  label1 = new QLabel();
  label1->setText(tr("    Range (\u0025):"));

  custom_plot = new QCustomPlot();

  label2 = new QLabel();
  label2->setText(tr("    Time (s):"));

  label3 = new QLabel();
  label3->setAlignment(Qt::AlignCenter);
  label3->setText(tr("\u2013"));

  min_y = new QDoubleSpinBox();
  min_y->setRange(-100,0);
  min_y->setDecimals(0);
  min_y->setSingleStep(1.0);
  min_y->setValue(-100);

  max_y = new QDoubleSpinBox();
  max_y->setRange(0,100);
  max_y->setDecimals(0);
  max_y->setSingleStep(1.0);
  max_y->setValue(100);

  domain = new QSpinBox();
  domain->setValue(10); // initialized the graph to show 10 seconds of data
  domain->setRange(0, 90);

  plot_visible_layout = new QGridLayout();
  plot_visible_layout->addWidget(new QLabel("Center:"), 0, 1, Qt::AlignCenter);
  plot_visible_layout->addWidget(new QLabel("Offset:"), 0, 2, Qt::AlignCenter);
  plot_visible_layout->addWidget(new QLabel("Counts per\ndivision:"),
    0, 3, Qt::AlignCenter);

  bottom_control_layout = new QHBoxLayout();
  bottom_control_layout->addWidget(pause_run_button, 1);
  bottom_control_layout->addWidget(label1, 0, Qt::AlignRight);
  bottom_control_layout->addWidget(min_y, 0);
  bottom_control_layout->addWidget(label3, 0);
  bottom_control_layout->addWidget(max_y, 0);
  bottom_control_layout->addWidget(label2, 0, Qt::AlignRight);
  bottom_control_layout->addWidget(domain, 0);

  setup_plot(input, "Input", "#00ffff", false, 4095);

  setup_plot(target, "Target", "#0000ff", false, 4095, true);

  setup_plot(feedback, "Feedback", "#ffc0cb", false, 4095);

  setup_plot(scaled_feedback, "Scaled feedback", "#ff0000", false, 4095, true);

  setup_plot(error, "Error", "#9400d3", true, 4095);

  setup_plot(integral, "Integral", "#ff8c00", true, 0x7fff);

  setup_plot(duty_cycle_target, "Duty cycle target", "#32cd32", true, 600);

  setup_plot(duty_cycle, "Duty cycle", "#006400", true, 600);

  setup_plot(raw_current, "Raw current (mV)", "#660066", false, 4095);

  setup_plot(current, "Current (mA)", "#b8860b", false, 100000);

  setup_plot(current_chopping, "Current chopping", "#ff00ff", false, 1);

  plot_visible_layout->addWidget(show_all_none, row, 0, 1, 3);

  QSharedPointer<QCPAxisTickerFixed> x_axis_ticker(new QCPAxisTickerFixed);
  x_axis_ticker->setTickStepStrategy(QCPAxisTicker::tssReadability);
  x_axis_ticker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);
  custom_plot->xAxis->setTicker(x_axis_ticker);

  QSharedPointer<QCPAxisTickerText> y_axis_ticker(new QCPAxisTickerText);

  y_axis_ticker->addTick(0, "0");

  for (int i = 10; i <= 100; (i += 10))
  {
    QString pos = QString::number(i);
    QString neg = QString::number(-i);
    y_axis_ticker->addTick(i, pos + "%");
    y_axis_ticker->addTick(-i, neg + "%");
  }

  custom_plot->yAxis->setTicker(y_axis_ticker);

  custom_plot->yAxis->setRange(-100,100);
  custom_plot->yAxis->setTickLabelPadding(10);
  custom_plot->xAxis->setTickLabelPadding(10);

  // this is used to see the x-axis to see accurate time.
  // custom_plot->xAxis2->setVisible(true);

  set_line_visible();

  QMetaObject::connectSlotsByName(this);
}

void graph_widget::setup_plot(plot& plot, QString display_text, QString color,
  bool signed_range, double range, bool default_visible)
{
  plot.color = color;

  plot.range_value = range;

  plot.range = new QDoubleSpinBox();
  plot.range->setDecimals(0);
  plot.range->setSingleStep(1.0);
  plot.range->setRange(0, plot.range_value);
  plot.range->setValue(plot.range_value);

  plot.center_value = new QDoubleSpinBox();
  plot.center_value->setDecimals(0);
  plot.center_value->setSingleStep(1.0);

  if (signed_range)
    plot.center_value->setRange(-plot.range_value, plot.range_value);
  else
    plot.center_value->setRange(0, plot.range_value);

  plot.center_value->setValue(0);

  plot.display = new QCheckBox();
  plot.display->setText(display_text);
  plot.display->setStyleSheet("border: 2px solid "+ color + ";"
    "padding: 2px;"
    "background-color: white;");
  plot.display->setCheckable(true);
  plot.display->setChecked(default_visible);

  plot.default_visible = default_visible;

  plot.range_label = new QLabel();

  if (signed_range)
    plot.range->setPrefix("\u00B1 ");
  else
    plot.range->setPrefix("\u002B ");

  plot.axis = custom_plot->axisRect(0)->addAxis(QCPAxis::atRight);
  plot.axis->setVisible(false);
  plot.axis->setRange(-plot.range_value, plot.range_value);

  plot.division_size = new QLabel();

  calculate_division_size(plot);

  show_all_none = new QPushButton("Show all/none");
  show_all_none->setObjectName("show_all_none");

  connect(show_all_none, SIGNAL(clicked()),
    this, SLOT(show_all_none_clicked()));

  plot_visible_layout->addWidget(plot.display, row, 0);
  plot_visible_layout->addWidget(plot.center_value, row, 1);
  plot_visible_layout->addWidget(plot.range, row, 2);
  plot_visible_layout->addWidget(plot.division_size, row, 3);

  plot.graph = custom_plot->addGraph(custom_plot->xAxis2, plot.axis);
  plot.graph->setPen(QPen(plot.color));

  connect(plot.range, SIGNAL(valueChanged(double)),
    this, SLOT(change_ranges()));

  connect(plot.center_value, SIGNAL(valueChanged(double)),
    this, SLOT(change_ranges()));

  connect(plot.display, SIGNAL(clicked()), this, SLOT(set_line_visible()));

  all_plots.append(&plot);

  row++;
}

// modifies the x-axis based on the domain value
// and removes data outside of visible range
void graph_widget::remove_data_to_scroll(uint32_t time)
{
  key = time; // stores a local copy of time value

  custom_plot->xAxis->setRange(-domain->value() * 1000, 0);

  custom_plot->xAxis2->setRange(time, domain->value() * 1000, Qt::AlignRight);

  for (auto plot : all_plots)
    plot->graph->data()->removeBefore(domain->value() * 1000);

  custom_plot->replot();
}

double graph_widget::calculate_division_size(plot& plot)
{
  double division_value = (plot.range->value() - plot.center_value->value())/10.0;

  if (division_value >= 100)
  {
    plot.division_size->setText("\u223c " + QString::number(((division_value)), 'f', 0));
  }
  else
    plot.division_size->setText("\u223c " + QString::number(((division_value)), 'f', 1));
}

void graph_widget::change_ranges()
{
  custom_plot->xAxis->setRange(-domain->value() * 1000, 0);

  custom_plot->xAxis2->setRange(key, domain->value() * 1000, Qt::AlignRight);

  for (auto plot : all_plots)
  {
    plot->axis->setRangeLower((plot->range->value() * (min_y->value()/100.0)) + plot->center_value->value());
    plot->axis->setRangeUpper((plot->range->value() * (max_y->value()/100.0)) + plot->center_value->value());

    calculate_division_size(*plot);
  }

  custom_plot->yAxis->setRange(min_y->value(), max_y->value());
  custom_plot->replot();
}

void graph_widget::on_pause_run_button_clicked()
{
  set_paused(!graph_paused);
}

void graph_widget::set_line_visible()
{
  for (auto plot : all_plots)
  {
    plot->graph->setVisible(plot->display->isChecked());
  }

  custom_plot->replot();
}

void graph_widget::show_all_none_clicked()
{
  bool all_checked = std::all_of(all_plots.begin(), all_plots.end(),
    [](const plot * plot) {return plot->display->isChecked();});

  for (auto plot : all_plots)
  {
    if (all_checked)
      plot->display->setChecked(false);
    else
      plot->display->setChecked(true);
  }

  set_line_visible();
}
