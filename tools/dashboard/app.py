# Streamlit dashboard for real-time monitoring

import streamlit as st
import pandas as pd
import plotly.express as px
from datetime import datetime
import time

from ..db import Database
from ..services.meter_service import MeterService
from ..config import Config

# Page config
st.set_page_config(
    page_title="Meter Reader Monitor",
    page_icon="📊",
    layout="wide"
)

# Initialize (singleton)
@st.cache_resource
def get_database():
    return Database()

@st.cache_resource
def get_meter_service():
    return MeterService(get_database())

# Main app
def main():
    st.title("📊 Meter Reader Real-Time Monitor")
    st.markdown("**Production monitoring dashboard for water and electric meters**")
    
    # Sidebar
    st.sidebar.header("⚙️ Settings")
    
    auto_refresh = st.sidebar.checkbox("Auto-refresh", value=True)
    refresh_interval = st.sidebar.slider("Refresh interval (seconds)", 5, 300, 10)
    
    stale_threshold = st.sidebar.number_input(
        "Stale threshold (minutes)",
        min_value=5,
        max_value=60,
        value=Config.STALE_THRESHOLD_MINUTES
    )
    
    # Auto-refresh
    if auto_refresh:
        time.sleep(refresh_interval)
        st.rerun()
    
    # Latest readings
    st.header("💧 Latest Readings")
    
    service = get_meter_service()
    readings = service.get_latest_readings()
    
    if readings:
        # Display as cards
        cols = st.columns(len(readings))
        for idx, reading in enumerate(readings):
            with cols[idx % len(cols)]:
                st.metric(
                    label=f"{reading.meter_id}",
                    value=f"{reading.value:.3f} {reading.unit}",
                    delta=None
                )
                st.caption(f"Updated: {reading.timestamp.strftime('%H:%M:%S')}")
    else:
        st.warning("No readings yet")
    
    # Meter health
    st.header("🏥 Meter Health Status")
    
    if readings:
        meter_ids = [r.meter_id for r in readings]
        selected_meter = st.selectbox("Select meter", meter_ids)
        
        if selected_meter:
            status = service.get_meter_status(selected_meter, stale_threshold)
            
            # Status indicator
            if status.is_healthy():
                st.success(f"✅ {status.meter_id}: {status.status}")
            else:
                st.error(f"⚠️ {status.meter_id}: {status.status} ({status.minutes_since_last} min ago)")
            
            # Hourly chart
            hourly_data = service.get_hourly_consumption(selected_meter, hours=24)
            
            if hourly_data:
                df = pd.DataFrame(hourly_data)
                fig = px.line(
                    df,
                    x='hour',
                    y='avg_value',
                    title=f"{selected_meter} - Last 24 Hours",
                    labels={'hour': 'Time', 'avg_value': f"Consumption ({df['unit'].iloc[0]})"}
                )
                st.plotly_chart(fig, use_container_width=True)
    
    # Footer
    st.markdown("---")
    st.caption(f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")


if __name__ == "__main__":
    main()