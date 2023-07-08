import HomeContainer from "@/containers/home";
import SubmissionFormContainer from "@/containers/submit";
import "@/styles/Home.module.css";

export default function Home() {
  return (
    <div
      style={{
        margin: "0 auto",
        width: "1000px",
      }}
    >
      <h1
        style={{
          fontFamily: "Courier",
          fontWeight: "bold",
          fontStyle: "italic",
          fontSize: "50px",
          textShadow: "#A3A3A3 1px 1px 1px",
        }}
      >
        <span>
          <span style={{ display: "inline-block" }}>
            <span style={{ color: "#ff0000" }}>M</span>
            <span style={{ color: "#ff6e00" }}>a</span>
            <span style={{ color: "#ffdd00" }}>n</span>
            <span style={{ color: "#b2ff00" }}>a</span>
            <span style={{ color: "#48ff00" }}>r</span>
            <span style={{ color: "#00ff26" }}>i</span>
            <span style={{ color: "#00ff95" }}>m</span>
            <span style={{ color: "#00fbff" }}>o&nbsp;</span>
          </span>
          <span style={{ display: "inline-block" }}>
            <span style={{ color: "#0091ff" }}>P</span>
            <span style={{ color: "#0022ff" }}>o</span>
            <span style={{ color: "#4d00ff" }}>r</span>
            <span style={{ color: "#b700ff" }}>t</span>
            <span style={{ color: "#ff00d9" }}>a</span>
            <span style={{ color: "#ff006a" }}>l</span>
          </span>
        </span>
      </h1>
      <SubmissionFormContainer />
      <HomeContainer />
    </div>
  );
}
