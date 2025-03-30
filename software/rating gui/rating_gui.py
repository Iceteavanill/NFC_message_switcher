import streamlit as st
import pandas as pd
from datetime import datetime
import os


# configuration area
unwanted_cols_in_uploaded_dataset = ['Email',
                                     'Start time',
                                     'Completion time',
                                     'Last modified time',
                                     'Ich nehme am 16. April am E-Abend teil',
                                     'Fleisch / Vegetarisch',
                                     'Optional : Ich mag dunkles Bier',
                                     'ID']

wanted_cols_in_uploaded_dataset = ["Start competition time",
                                   "Finish Time",
                                   "Duration (s)",
                                   "Comments",
                                   "Score",
                                   "Finishing order"]

rating_points_bool = ["Components straight", 
                      "Nothing burned", 
                      "No solder peaks", 
                      "Melf Stripes aligned" 
                      ]


@st.dialog("Add a new contestant")
def add_new_contestant():
    data_temp_storage = {}
    for col in data_frame.columns:
        if col not in wanted_cols_in_uploaded_dataset + rating_points_bool:
            data_temp_storage[col] = st.text_input(col)

    if st.button("Submit"):
        new_row = pd.DataFrame([data_temp_storage])
        st.session_state.data = pd.concat([st.session_state.data, new_row], ignore_index=True)
        st.rerun()

# check if Colums are prsent int the loaded CSV
def init_csv(data_frame):
    # removing unwanted cols
    for col in unwanted_cols_in_uploaded_dataset:
        if col in data_frame.columns:
             data_frame.drop(col, axis=1, inplace=True) 

    # adding wanted cols
    for col in wanted_cols_in_uploaded_dataset + rating_points_bool:
        if col not in data_frame.columns:
            data_frame[col] = None
    return data_frame

# Format display name
def format_name(row):
    return f"{row['Name']} ({row.get('Semester', '')})"

# init values that are used later
if "ignore_active" not in st.session_state:
    st.session_state.ignore_active = False
if "ignore_done" not in st.session_state:
    st.session_state.ignore_done = False


# Upload section
if "data" not in st.session_state:
    uploaded = st.file_uploader("Upload CSV", type="csv")
    if uploaded:
        data_frame = pd.read_csv(uploaded)
        data_frame = init_csv(data_frame)
        st.session_state.data = data_frame
        st.session_state.selected_index = None
        st.rerun()

# Load data from session
if "data" in st.session_state:
    data_frame = st.session_state.data

    if st.button("Add a new contestant"):
        add_new_contestant()

    # --- Sidebar: Checked-in contestants list (clickable buttons) ---
    with st.sidebar:
        st.write("# active contestants")
        checked_in = data_frame[data_frame["Start competition time"].notna() & data_frame["Finish Time"].isna()]
        if checked_in.empty:
            st.badge("No contestant having at it", color="violet", icon=":material/info:")            
        else:
            for idx, row in checked_in.iterrows():
                if st.button(format_name(row), key=f"sidebar_{idx}"):
                    st.session_state.selected_index = idx
                    st.rerun()
    # --- Main UI ---
    st.write("## Contestant Evaluation")


    # --- Search section ---
    search = st.text_input("Search by name").strip().lower()
    if search:
        matches = data_frame[data_frame["Name"].str.lower().str.contains(search)]
        if not st.session_state["ignore_done"]:
            matches = matches[matches["Finish Time"].isna()]

        if not st.session_state["ignore_active"]:
            matches = matches[matches["Start competition time"].isna()]

        if not matches.empty:
            names = matches.apply(format_name, axis=1)
            selected_label = st.selectbox("Select contestant", names)
            st.session_state.selected_index = matches[names == selected_label].index[0]
        else:
            st.warning("No matching contestants found.")
    
    col1, col2, col3 = st.columns(3)

    # Reset search selection
    with col1:
        if st.button("Clear selection"):
            st.session_state.selected_index = None

    with col2:
        st.session_state.ignore_active = st.checkbox("Ignoren't busy people") # Ignoren't = dont ignore (is an insider)


    with col3:
        st.session_state.ignore_done = st.checkbox("Ignoren't finished people") # Ignoren't = dont ignore (is an insider)

    # --- Display contestant if selected ---
    if st.session_state.selected_index is not None:
        i = st.session_state.selected_index
        person = data_frame.loc[i]
        
        st.write("### Selected contestant : "+ person["Name"])
        st.dataframe(data_frame.loc[data_frame["Name"] == person["Name"]], column_order=["Name", "Semester"])

        if pd.isna(person["Start competition time"]):
            if st.button("Start Timer", key="checkin"):
                now = datetime.now()
                data_frame.at[i, "Start competition time"] = now.isoformat()
                st.success(f"Checked in at {now.strftime('%H:%M:%S')}")
                st.rerun()
        else:
            checkin_time = pd.to_datetime(person["Start competition time"])
            subcol1, subcol2 = st.columns([3,1])
            with subcol1:
                st.info(f"Checked in at {checkin_time.strftime('%H:%M:%S')}")

            with subcol2:
                if st.button("reset timer"):
                    # reset Button
                    data_frame.at[i, "Start competition time"] = pd.NA
                    data_frame.at[i, "Finish Time"] = pd.NA
                    st.rerun()

            # Evaluation inputs
            st.write("### Rating")
            rating_items_value = {}
            for rating_point in rating_points_bool:
                rating_items_value[rating_point] = st.checkbox(rating_point, value=bool(person[rating_point]))
            score = st.slider("Overal rating", 0, 100,
                                   value=int(person["Score"]) if pd.notna(person["Score"]) else 50)
            comments = st.text_area("Comments", value=person["Comments"] or "")

            if st.button("Submit Rating & Stop Timer"):
                now = datetime.now()
                duration = int((now - checkin_time).total_seconds())
                data_frame.at[i, "Finish Time"] = now.isoformat()
                data_frame.at[i, "Duration (s)"] = duration
                for rating_point in rating_points_bool:
                    data_frame.at[i, rating_point] = rating_items_value[rating_point]
                data_frame.at[i, "Score"] = score
                data_frame.at[i, "Comments"] = comments 
                data_frame.at[i, "Finishing order"], _ = data_frame[data_frame["Finish Time"].notna()].shape

                if not os.path.isdir("backup"):
                    os.makedirs("backup")
                with open("backup/operations.csv", "a") as file_bk:
                    data_to_write = [data_frame.at[i, "Name"], data_frame.at[i, "Semester"]] +\
                                    [rating_items_value[val] for val in rating_points_bool] +\
                                    [duration, score]
                                    
                    file_bk.write(";".join(str(x) for x in data_to_write) + '\n')
                
                st.success(f"Rating saved. Duration: {duration} seconds.")
                st.session_state.selected_index = None
                st.rerun()

    # --- Export section ---
    st.write("### Export")
    st.download_button("Download ratings CSV", data_frame.to_csv(index=False), "ratings.csv", "text/csv")

    st.write("### Raw data view")
    st.dataframe(data_frame)