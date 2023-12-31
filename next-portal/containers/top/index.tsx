import Image from "next/image";
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
      <tbody>
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
              <Image
                width={400}
                height={200}
                src={`/public/problem_images/${problem.id}.svg`}
                alt=""
              />
            </td>
          </tr>
        ))}
      </tbody>
    </table>
  );
};
export default HomeContainer;
