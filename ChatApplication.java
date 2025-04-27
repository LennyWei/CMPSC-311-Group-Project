package com.example.javaFX;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;
import javafx.geometry.Insets;
import javafx.util.Pair;

import java.io.*;
import java.net.Socket;
import java.util.Optional;

public class ChatApplication extends Application {
    private VBox messageBox;
    private ScrollPane scrollPane;
    private TextField inputField = new TextField();
    private Button sendButton = new Button("Send");
    private String ipAddress;
    private String portNumber;

    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    @Override
    public void start(Stage stage) {
        messageBox = new VBox(10);
        messageBox.setPadding(new Insets(10));
        scrollPane = new ScrollPane(messageBox);
        scrollPane.setFitToWidth(true);
        inputField.setPromptText("Type your message...");

        HBox inputBox = new HBox(10, inputField, sendButton);

        VBox root = new VBox(10, scrollPane, inputBox);
        root.setPrefSize(700, 400);
        Scene scene = new Scene(root);
        scene.getRoot().setStyle("-fx-font-family: 'Helvetica';");

        stage.setScene(scene);
        stage.setTitle("JavaFX Chat Client");
        stage.show();

        Dialog<Pair<String, String>> dialog = new Dialog<>();
        dialog.setTitle("Setup");
        dialog.setHeaderText("Connection Information and Username");
        dialog.getDialogPane().getButtonTypes().addAll(ButtonType.OK, ButtonType.CANCEL);
        GridPane grid = new GridPane();
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(20, 150, 10, 10));
        TextField connectionInfo = new TextField();
        connectionInfo.setPromptText("IP:Port Number");
        TextField username = new TextField();
        username.setPromptText("Username");

        grid.add(new Label("Connection Information:"), 0, 0);
        grid.add(connectionInfo, 1, 0);
        grid.add(new Label("Username:"), 0, 1);
        grid.add(username, 1, 1);

        dialog.getDialogPane().setContent(grid);
        dialog.setResultConverter(dialogButton -> {
            return new Pair<>(connectionInfo.getText(), username.getText());
        });
        Optional<Pair<String, String>> result = dialog.showAndWait();
        result.ifPresent(setupInfo -> {
            String[] splitAddress = setupInfo.getKey().split(":");
            ipAddress = splitAddress[0];
            portNumber = splitAddress[1];
            sendButton.setOnAction(e -> sendMessage(setupInfo.getValue()));
            System.out.println("ConnectionInfo=" + setupInfo.getKey() + ", Username=" + setupInfo.getValue());
        });

        new Thread(() -> connect(ipAddress, portNumber)).start();

    }

    private void connect(String ip, String port) {
        try {
            socket = new Socket(ip, Integer.parseInt(port));
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            receiveMessages();
//            new Thread(this::receiveMessages).start();
        } catch (IOException e) {
            System.out.println("fail");
//            chatArea.appendText("Failed to connect to server.\n");
        }
    }

    private void sendMessage(String username) {
        String msg = inputField.getText();
        if (!msg.isEmpty() && out != null) {
            out.println("[" + username + "] " + msg);
            inputField.clear();
        }
    }

    private void receiveMessages() {
        try {
            String msg;
            while ((msg = in.readLine()) != null) {
                String finalMsg = msg;
                javafx.application.Platform.runLater(() ->{
                    Label messageLabel = new Label(finalMsg);
                    messageLabel.setWrapText(true);
                    messageLabel.setStyle("-fx-background-color: lightblue; -fx-padding: 10px; -fx-background-radius: 10px;");

                    messageBox.getChildren().add(messageLabel);
                });

//                javafx.application.Platform.runLater(() -> chatArea.appendText(finalMsg + "\n"));
            }
        } catch (IOException e) {
            System.out.println("error");
//            javafx.application.Platform.runLater(() -> chatArea.appendText("Connection closed.\n"));
        }
    }

    public static void main(String[] args) {
        launch();
    }
}
