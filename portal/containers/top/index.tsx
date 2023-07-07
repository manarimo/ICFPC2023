import useProblemList from "./hooks/useProblemList";
import "@/styles/Home.module.css";

const HomeContainer = () => {
  const problems = useProblemList();
  if (!problems.data) {
    return <div>loading...</div>;
  }
  return (
    <table
      style={{
        border: "1px solid black",
        borderCollapse: "collapse",
      }}
    >
      {problems.data.map((problem) => (
        <tr
          key={problem.id}
          style={{
            maxHeight: "200px",
          }}
        >
          <th style={{ border: "1px solid black" }}>{problem.id}</th>
          <td
            style={{
              border: "1px solid black",
              padding: "5px",
            }}
          >
            {problem.roomWidth} x {problem.roomHeight}
          </td>
          <td
            style={{
              border: "1px solid black",
              padding: "5px",
            }}
          >
            <ul>
              <li>Musicians: {problem.numMusicians}</li>
              <li>Attendees: {problem.numAttendees}</li>
            </ul>
          </td>
          <td
            style={{
              border: "1px solid black",
              padding: "5px",
            }}
          >
            <img
              style={{
                width: "400px",
                maxHeight: "200px",
              }}
              src="/problems/images/1.svg"
            />
          </td>
        </tr>
      ))}
    </table>
  );
};
export default HomeContainer;
